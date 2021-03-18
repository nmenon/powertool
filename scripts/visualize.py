#!/usr/bin/env python3
# (C) Copyright 2016
# Texas Instruments, <www.ti.com>
# Nishanth Menon <nm@ti.com>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation version 2.
#
# This program is distributed .as is. WITHOUT ANY WARRANTY of any kind,
# whether express or implied; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA

"""Plot power measurement data


Usage: python visualize.py [options]

Options:
-t ..., --title=...     Use title of the plot as provided
-h, --help              show this help
-p, --power             Plot the power consumption data
-c, --current           Plot the current consumption data
-r, --rail_voltage      Plot the rail voltage data
-s, --shunt_voltage     Plot the shunt voltage data
-a ..., --algo=...      Input data format (currently just dump)
-i ..., --interval=...  Use the value in ms as refresh interval
"""

import sys
import six
import getopt
import time
import threading
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as anim
from matplotlib import colors

class Rail:
    name="unknown"
    shunt_uVs = []
    rail_mVs = []
    current_mAs = []
    power_mWs = []
    time_stamps = []
    plot = None
    anim = None

    def __init__(self, name):
        self.name=name
        self.shunt_uVs = np.array([])
        self.rail_mVs = np.array([])
        self.current_mAs = np.array([])
        self.power_mWs = np.array([])
        self.time_stamps = np.array([])

    def add_data(self, ts, shunt_uV, rail_uV, current_mA, power_mW):
        self.time_stamps = np.append(self.time_stamps, float(ts))
        self.shunt_uVs = np.append(self.shunt_uVs, float(shunt_uV))
        self.rail_mVs = np.append(self.rail_mVs, float(rail_uV) / 1000)
        self.current_mAs = np.append(self.current_mAs, float(current_mA))
        self.power_mWs = np.append(self.power_mWs, float(power_mW))

class Panel:
    name = "Unknown"
    dtype = "Unknown"
    cidx = 0

    interval = 500

    def __init__(self, name, displ_type, display_desc, interval):
        self.name = name
        self.dtype = displ_type
        self.interval = interval

        self.fig = plt.figure()
        self.fig.canvas.set_window_title(name)

        self.fig.subplots_adjust(left=0.09, bottom=0.09, right=1, top=0.95)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_ylabel(display_desc)

        c_ = list(six.iteritems(colors.cnames))
        self.c_hex = [c[1] for c in c_]


    def update_plot(caller, rail, panel):
        show_list = [  ]
        if panel.dtype == "power":
            show_list = rail.power_mWs
        if panel.dtype == "shuntv":
            show_list = rail.shunt_uVs
        if panel.dtype == "railv":
            show_list = rail.rail_mVs
        if panel.dtype == "current":
            show_list = rail.current_mAs
        rail.plot.set_data(rail.time_stamps, show_list)
        panel.ax.relim()
        panel.ax.legend()
        panel.ax.autoscale_view(tight=False, scalex=True, scaley=True)
        panel.ax.autoscale(enable=True, axis='both', tight=True)
        panel.ax.set_xlabel('Sampling duration')
        panel.ax.set_title(panel.name)
        plt.draw()
        return rail.plot,

    def display_rail(self, rail):

        show_list = [  ]
        if self.dtype == "power":
            show_list = rail.power_mWs
        if self.dtype == "shuntv":
            show_list = rail.shunt_uVs
        if self.dtype == "railv":
            show_list = rail.rail_mVs
        if self.dtype == "current":
            show_list = rail.current_mAs

        cs = self.c_hex[self.cidx]

        ax = self.ax
        rail.plot, = ax.plot(rail.time_stamps,
                            show_list,
                            color=cs,
                            alpha=0.8,
                            label = rail.name,
                            linewidth=5)

        self.cidx =  self.cidx + 1

    def show(self):
        plt.show()


class DataReader(object):
    section = "-----8<-------"

    def __init__(self, p):
        self.interval = p.interval
        self.p = p
        self.ts = -1
        self.thread = threading.Thread(target=self.run, args=())
        self.thread.daemon = True
        self.thread.start()

    def run(self):
        while True:
            for line in sys.stdin:
                line = line.rstrip()
                line = line.lstrip()
                if line == self.section:
                    continue
                rdata = [edata.strip() for edata in line.split('|')]
                ridx=0
                self.ts = self.ts + 1
                for r in rdata:
                    rinfo = [edata.strip() for edata in r.split(',')]
                    # Empty entry
                    if rinfo[0] == '':
                        continue
                    rail = Rails[ridx]
                    ridx = ridx + 1
                    rail.add_data(self.ts,
                                  rinfo[0],
                                  rinfo[1],
                                  rinfo[2],
                                  rinfo[3])
                    self.p.update_plot(rail, self.p)
            time.sleep(self.interval)


def Collect_stream_data(p):
    section = "-----8<-------"
    rail_desc="Rail Listing"
    data_start="Data Dump Start"
    # state is dump data
    state = 0

    for line in sys.stdin:
        line = line.rstrip()
        line = line.lstrip()

        if line == section:
            continue;
        elif line == rail_desc:
            state = 1
            continue;
        elif line == data_start:
            data_reader = DataReader(p)
            return
        elif state == 1:
            rdata = [edata.strip() for edata in line.split('|')]
            for r in rdata:
                rinfo = [edata.strip() for edata in r.split(',')]
                next_rail = Rail(rinfo[0])
                Rails.append(next_rail)
                p.display_rail(next_rail)
            continue


def Collect_dump_data(p):
    section = "-----8<-------"
    data_start="Data Dump Start"
    # state is dump data
    state = 0

    ya = 10
    for line in sys.stdin:
        line = line.rstrip()
        line = line.lstrip()

        if line == section:
            # if state machine was collecting data for prev rail, we have new rail
            if state > 1:
                state = 1
            continue;

        if line == data_start:
            # state is now set to get the data string
            state = 1
            continue

        if state == 0:
            # Still waiting for data
            continue

        sdata = [edata.strip() for edata in line.split(',')]

        # If we have a new rail.. state is 1
        if state == 1:
            next_rail = Rail(sdata[1])
            Rails.append(next_rail)
            p.display_rail(next_rail)
            # And start adding data to the rail
            state = 2
            continue

        # are we at point where we have documentation of feilds?
        if state == 2:
            state = 3
            continue

        # Data
        if state == 3:
            next_rail.add_data(sdata[0], sdata[1], sdata[2], sdata[3], sdata[4])

    for next_rail in Rails:
        p.update_plot(next_rail, p)

def usage():
    print (__doc__)

def main(argv):
    try:
        opts, args = getopt.getopt(argv, "hpsrct:i:a:", ["help", "power", "shunt_voltage", "rail_voltage", "current", "title=", "interval=", "algo="])
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    topic="Power Measurement Demonstration"
    t=""
    algo="dump"
    refresh_interval=500
    for opt, arg in opts:
        if opt in ( "-h" , "--help" ):
            usage()
            sys.exit()
        elif opt in ( "-p" , "--power" ):
            t = "power"
            desc="Power Consumption per sample (mW)"
        elif opt in ( "-s" , "--shunt_voltage" ):
            t = "shuntv"
            desc="Shunt Voltage(uV)"
        elif opt in ( "-r" , "--rail_voltage" ):
            t = "railv"
            desc="Rail Voltage(mV)"
        elif opt in ( "-c" , "--current" ):
            t = "current"
            desc="Current Consumption(mA)"
        elif opt in ( "-t" , "--title" ):
            topic = arg
        elif opt in ( "-a" , "--algo" ):
            algo = arg
        elif opt in ( "-i" , "--interval" ):
            refresh_interval = arg

    if t == "" :
        usage()
        sys.exit(1)

    p = Panel(topic, t, desc, refresh_interval)

    if algo == "dump":
        # We just have dump of data incoming
        Collect_dump_data(p)
    if algo == "stream":
        # We just have dump of data incoming
        Collect_stream_data(p)
    #All data available.. display.
    p.show()

Rails = []
if __name__ == "__main__":
    main(sys.argv[1:])
