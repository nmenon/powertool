powertool
=========

Utility to collect power measurements.

Background:
----------

Most TI development boards have shunt resistors on them to measure current draw
on key voltage rails. On many such boards high accuracy ADCs (INA226) are usually
mounted to allow capturing shunt voltage and provide direct digital readouts of
current and power without needing to modify the board and/or attach external lab
equipment. If your board only has shunt resistors and no INA226 devices then you
may use an off the shelf INA226 evaluation board.

The i2c bus is not connected to the DUT requiring a separate device to connect,
configure, and read the INA226 devices. This is useful as the DUT can be 1) in a
suspend mode, 2) running a non-Linux OS, or 3) under heavy load which may be
slowed or interrupted by a power measurment utility. As such this tool was
developed to use an external platform's i2c bus, either native or through FTDI
cable with MPSSE, to read power numbers from a board containing the DUT.

There are many sway to skin a cat and solutions like
http://www.totalphase.com/products/aardvark-i2cspi/ do exist in the market
which may do this job as well and may even be incorporated into this tool
similar to the support of the FTDI MPSSE option.

Make sure to see the WARRANTY and Support section below.

What can powertool do?
---------------------

* It can measure current and voltage and do averaging (if that is what you'd
  like it to do) and many other things that I wont put in README.md and keep
  it updated :D.
* It uses configuration file to read information about the measurement
  environment. So, adding support for a new board you want to measure is simply
  a new config file.
* It runs on Linux - but I think any posix OS might work. Tested on kernel v4.x
* It currently runs as a sandbox(test) on any linux box - PC or anywhere.
* It currently runs as measurement build on beaglebone-black or raspberry pi or
  any system with native I2C. 

How does powertool work?
---------------------

Powertool is a utility that you run natively on an ARM based SoC / single-board
computer such as the BeagleBone and Raspberry Pi. There it will use the native
i2c module. Powertool can also run on an x86 based PC and it will use an FTDI
cable as the USB-to-I2C converter.

Option 1: use a development board (example beaglebone black)
```
         (**)ptool runs here
                                BeagleBone Black(**)
  o      +-------+    uart     +------------------+
 -+-     |       |    cable    |                  |
  |      |       +---------->  +------------------+ P9 expansion connector
  +      |       |             +------------------+ P9.17 (SCL) / P9.18 (SDA)
 / \     +-------+                  |     |    |
          Your PC                   | I2C |    |
 You      serial             +--+---+     |    | GND
 :)       utility            |  |---------+    |
          (minicom)      +---|--|--------------|---------------+
                         |   |  |              v               |
                         |   |  |                              |
                         |   |  |          +-------------+     |
                         |   |  |          |  Sink for   |     |
                         |   |  |          |  Supply     |     |
                         |   |  |          +------^------+     |
                         |   |  |                 |            |
                         |   v  v    +----------> |  (-)       |
                         |           |            |            |
                         | +---------+      +-----^-----+      |
                         | | INA226  |      |  Shunt    |      |
                         | |         |      |  Resistor |      |
                         | +---------+      +-----^-----+      |
                         |           |            |            |
                         |           +----------> |  (+)       |
                         |                        |            |
                         |                +-------^------+     |
                         |                |  Source of   |     |
                         |                |  Supply      |     |
                         |                +--------------+     |
                         |                                     |
                         +-------------------------------------+
                                  Board under measurement
```

Option 2: use an FTDI chip with MPSSE (ex: FT2232H Mini Module[1], C232HM-DDHSL-0[2]
  [1] http://www.ftdichip.com/Support/Documents/DataSheets/Modules/DS_FT2232H_Mini_Module.pdf,
  [2] http://www.ftdichip.com/Products/Cables/USBMPSSE.htm)
```
         (**)ptool runs here
                                FTDI chip with MPSSE
  o      +-------+    USB      +------------------+
  +      |       |    cable    |                  |
 /|\     |       +---------->  +------------------+
  +      |       |             +------------------+ (see doc/board_details.txt
 / \     +-------+                  |     |    |       for wiring)
          Your PC                   | I2C |    |
 You      Terminal(**)       +--+---+     |    | GND
 :)                          |  |---------+    |
                         +---|--|--------------|---------------+
                         |   |  |              v               |
                         |   |  |                              |
                         |   |  |          +-------------+     |
                         |   |  |          |  Sink for   |     |
                         |   |  |          |  Supply     |     |
                         |   |  |          +------^------+     |
                         |   |  |                 |            |
                         |   v  v    +----------> |  (-)       |
                         |           |            |            |
                         | +---------+      +-----^-----+      |
                         | | INA226  |      |  Shunt    |      |
                         | |         |      |  Resistor |      |
                         | +---------+      +-----^-----+      |
                         |           |            |            |
                         |           +----------> |  (+)       |
                         |                        |            |
                         |                +-------^------+     |
                         |                |  Source of   |     |
                         |                |  Supply      |     |
                         |                +--------------+     |
                         |                                     |
                         +-------------------------------------+
                                  Board under measurement
```

Building the powertool
----------------------

If you cloned this repo on an x86 based computer but plan to run ptool on ARM.

1. Make sure an ARM compiler is installed

	sudo apt install crossbuild-essential-armhf

2. Cross Compiling:

	make CROSS_COMPILE=arm-linux-gnueabihf-

If you cloned this repo on an ARM based computer to run natively.

1. Make sure a native compiler is installed

	sudo apt install build-essential

2. Native build:

	make

If you cloned this repo on an x86 based computer to run natively / over FTDI.

1. Make sure a native compiler and the FTDI Dev Library are installed

	sudo apt install build-essential libftdi-dev

2. Native build:

	make I2C=mpsse

If you plan to test the application without measuring on a real board. There may
be many reasons you'd like this: you dont have a board and want to give
powertool features a feel; you'd like to write an enhacement to powertool and
want to run valgrind or debug with gdb that your final platform wont let you..
basically sandbox is pretty much powertool except it generates random data
instead of actual measurements.

1. Building a sandbox:

	make ARCH=sandbox

At the end of the build, you get 'ptool'

What else can make do? just run:
	make help

How to run powertool?
---------------------

Simplest command (default 100 captures, 200 ms between):

	./ptool -c configs/board.conf -e

Custom command (execute 15 captures, 100 ms between):

	./ptool -c configs/board.conf -e -n 15 -s 100

How is powertool source organized?
---------------------

As of this writing (current revision 0.1), this is how it looks like:
```
	├── configs - place where all board config files are
	├── docs - details on connections and misc stuff
	├── include - general includes
	├── kernel-patches - uggh.. should be empty, but, at times..
	├── lib - third party libraries are organized in directories
	│   ├── i2c-tools
	│   │   ├── include
	│   │   │   └── linux
	│   │   └── tools
	│   ├── lcfg
	│   └── omapconf
	│       └── common
	└── src - everything source
	    └── powertool - here we are.. and start at main.c
	    if you add new tools, we'd have it here.
```

Why is it written in C?
----------------------
I like C over python, C++, java.. ruby.... etc..

Hardware setup:
--------------

For using this tool as it is, setup is simple: Beaglebone - i2c - INA
on board (or seperate board) - shunt resistor on voltage rail. Follow
the wiring as described in docs/board_details.txt for each of the
supported boards that have configs/ config file.

Writing support for a new board:
-------------------------------
It can't get easier than this:

```
	busName = {
		/*
		 * BeagleBoneBlack Bus mapping - Should not change if you wire it right.
		 * if changed, then change this mapping
		 */
		i2c = "1"

		rail_name = {
			/* This represents 1 INA meauring a specific voltage rail */
			group	= "measurement group"
			address	= "INA's i2c address in hex"
			input	= "What is the input to the shunt (the + side of INA measurement)"
			output	= "What is the output of the shunt (the - side of INA measurement)"
			shunt_value	= "What is the shunt resistor value used in Ohms"
			shunt_accuracy	= "How accurate is the resistor in %"
		}
	}
```

Using mpsse, build with I2C=mpsse and use a configuration like the following:
```
	busName = {
		/*
		 * When using a FT2232H Mini Module over MPSSE interface
		 * Format: vendorID:productID:interface:serialNumber
		 * use lsusb -v to see the details
		 */
		i2c = "0x0403:0x6010:2:FTVFS9V0"

		rail_name = {
			/* This represents 1 INA meauring a specific voltage rail */
			group	= "measurement group"
			address	= "INA's i2c address in hex"
			input	= "What is the input to the shunt (the + side of INA measurement)"
			output	= "What is the output of the shunt (the - side of INA measurement)"
			shunt_value	= "What is the shunt resistor value used in Ohms"
			shunt_accuracy	= "How accurate is the resistor in %"
		}
	}
```

See configs/example_ftdi.conf for detailed example

How to contribute / I have this nice idea to improve and share?
-----------------

https://github.com/nmenon/powertool

Yes! If you are reading this section, I am happy! :). I wrote this tool in
around a couple of days. It is no close to being complete. You can:
- Download and give it a shot - even sandbox version
- Read code and provide fixes - grammatical or spelling fixes are nice to get.
- Support your own environment world - new board config files are nice.
- See the hard things in TODO which I never got around to doing, help fix them
- Write new features that you might like it to do.. NOTE: lets not
make it powertop replacement.

What ever you want to do, do it :). and send me github pull requests
once you are done and feel like it. Just remember to check out
LICENSING for your contributions, I'd like to continue to keep it
GPLV2.

There is no mailing lists at the moment, no IRC channels either. But,
I usually lurk around in a few channels on freenode.net, so feel free
to ping me if you need.

WARRANTY and Support, Questions, Training:
-------------------------------------------------

This is NOT a tool written as an "official" software delivery from either me or
my employer. This is NOT supported in any form official or unofficial. Check out
the 'How to contribute' section if you have bug fixes or improvments. Use GitHub
to file and track issues. For questions on TI Keystone, TI Sitara, or TI Jacinto
processors and evaluation boards use the Processors sub-forum at http://e2e.ti.com

