powertool
=========

Utility to collect power measurements.

Background:
----------

Certain boards have shunt resistors on them to measure current on
key voltage rails. On many such TI development boards, an INA226 is
usually mounted on a seperate i2c bus to allow external tools to
measure current, voltage etc.

This is useful, but it is a pain to get the tools to work
consistently. They are either built for windows OR expensive etc..
Instead, as developer with access to other development platforms, it
is very very trivial to use the 2nd development platform's i2c bus to
read power numbers from the board that is under evaluation.

Doing things in userspace and linux is so much easier and allows us to
do additional processing that pre-packaged utilities dont let us.

So, that was basically the history of what motivated this simple
utility.

NOTE: There are more than one way to skin a cat :). Solutions like
http://www.totalphase.com/products/aardvark-i2cspi/ do exist in the market
which may do this job as well and may even be incorporated in this tool
similar to the support of FTDI MPSSE option added.

What can powertool do?
---------------------
* It can measure current and voltage and do averaging (if that is what you'd
  like it to do) and many other things that I wont put in README.md and keep
  it updated :D.
* It uses configuration file to read information about the measurement
  environment. So, adding support for a new board you want to measure is a
  new config file.
* It runs on Linux - but I think any posix OS might work. Tested only on v3.15.
* It currently runs as a sandbox(test) on any linux box - PC or anywhere.
* It currently runs as measurement build on beaglebone-black (but no reason
  why it would be restricted there)

How does powertool work?
---------------------
Option 1: use a development board (example beaglebone black)
```
                                 (*)ptool runs here

                                 BeagleBone Black(*)
  o      +-------+    uart     +------------------+
 -+-     |       |    cable    |                  |
  |  +>  |       +---------->  +------------------+
  +      |       |             +------------------+ P9 expansion connector
/  \     +-------+                  |     |
             PC                     | I2C |
 You       serial            +-+----+     |
 :)       utility            | |----------+
          (minicom)      +----------------+--------------------+
                         |   | |                               |
                         |   | |           +-------------+     |
                         |   | |           |             |     |
                         |   | |           |  Sink for   |     |
                         |   | |           |  Supply     |     |
                         |   | |           +------+------+     |
                         |   | |                  |            |
                         |   v v     +----------> |  -         |
                         |           |            |            |
                         | +---------+      +-----+-----+      |
                         | | INA226  |      |  Shunt    |      |
                         | |         |      |  Resistor |      |
                         | +---------+      +-----+-----+      |
                         |           |            |            |
                         |           +----------> |  +         |
                         |                        |            |
                         |                +-------+------+     |
                         |                |  Source of   |     |
                         |                |  Supply      |     |
                         |                +--------------+     |
                         |                                     |
                         +-------------------------------------+
                                  Board under measurement
```
Option 2: use an FT2232H Mini Module[1]
 http://www.ftdichip.com/Support/Documents/DataSheets/Modules/DS_FT2232H_Mini_Module.pdf

```
         (*)ptool runs here

                                 FT2232H Mini Module
  o      +-------+    USB      +------------------+
  +      |       |    cable    |                  |
 /|\ +>  |       +---------->  +------------------+
  +      |       |             +------------------+ CN2/CN3 connector
/  \     +-------+                  |     |
             PC                     | I2C |
 You      Terminal(*)        +-+----+     |
 :)                          | |----------+
                         +----------------+--------------------+
                         |   | |                               |
                         |   | |           +-------------+     |
                         |   | |           |             |     |
                         |   | |           |  Sink for   |     |
                         |   | |           |  Supply     |     |
                         |   | |           +------+------+     |
                         |   | |                  |            |
                         |   v v     +----------> |  -         |
                         |           |            |            |
                         | +---------+      +-----+-----+      |
                         | | INA226  |      |  Shunt    |      |
                         | |         |      |  Resistor |      |
                         | +---------+      +-----+-----+      |
                         |           |            |            |
                         |           +----------> |  +         |
                         |                        |            |
                         |                +-------+------+     |
                         |                |  Source of   |     |
                         |                |  Supply      |     |
                         |                +--------------+     |
                         |                                     |
                         +-------------------------------------+
                                  Board under measurement
```

ptool is a utility that you run on beaglebone black / PC and use it
to measure the current and voltage on another platform. It uses i2c-dev to
talk to the INA226 ADC, collect current and voltage values and finally
displays it to user.

In reality, you can run it anywhere and as long as you have an ADC,
an shunt mounted on the actual board, you can measure.

You can invoke ptool in any way you like: over serial port, ethernet
or what ever.

If your board only has shunt resistor and no INA226, then look for
one of those off the shelf INA226 devel boards.

Building the powertool
----------------------

1. Cross Compiling: It is pretty much similar to kernel

	make CROSS_COMPILE=arm-linux-gnueabi-

2. Native build:

	make

3. Building a sandbox (testing without actually wanting to do
measurements on real board). There may be many reasons you'd like
this: you dont have a board and want to give powertool features a
feel; you'd like to write an enhacement to powertool and want to run
valgrind or debug with gdb that your final platform wont let you..
etc.. basically sandbox is pretty much powertool except it generates
random data instead of actual measurement

	make ARCH=sandbox

4. Building with mpsse (FTDI bitbang example) - requires libftdi and libftdi-dev
installed:

	make I2C=mpsse

At the end of the build, you get 'ptool'

What else can make do? just run:
	make help

How to run powertool?
---------------------

Umm.... really? Since the tool keeps changing on my whim (and soon,
hopefully, your whim), I dont really want to keep this document
updated. Instead, the following should get you started.

	./ptool -?

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
the wiring as described in docs/board\_details.txt for each of the
supported boards that have configs/ config file.

Writing support for a new board:
-------------------------------
It cant get easier than this:

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

Folks who work on newer linux kernel will notice the similarity
of this with device tree. Yeah, you are right.. I should
probably start calling it ABI and write bindings document :P..
(poking-fun-at-device-tree, which happens to be a running joke among
we kernel folks in 2014)

Jokes apart, the logic is pretty similar to device tree, but not exactly the
same though - no ';' seperator, no phandles... but should be rather
self evident when you look at existing examples.

How to contribute / I have this nice idea to improve and share?
-----------------

https://github.com/nmenon/powertool

Yes! If you are reading this section, I am happy! :). I wrote this
tool in around a couple of days. It is no close to being complete. You can:
- Download and give it a shot - even sandbox version
- Read code and provide fixes - even grammatical or spelling fixes are
nice to get.
- Support your own environment world - new board config files are nice.
- See the hard things in TODO which I never got around to doing, help
fix them up
- Write new features that you might like it to do.. NOTE: lets not
make it powertop replacement.

What ever you want to do, do it :). and send me github pull requests
once you are done and feel like it. Just remember to check out
LICENSING for your contributions, I'd like to continue to keep it
GPLV2.

There is no mailing lists at the moment, no IRC channels either. But,
I usually lurk around in a few channels on freenode.net, so feel free
to ping me if you need.

WARRANTY, Where do I get support and training for this tool and other misc questions:
-------------------------------------------------

You are kidding, right? I want to make it clear: This is just a
personal itch I scratched. I might even not think about this tool
anymore a few years down the line.

If using this tool or resultant data or in any modified version,
you kill someone, get sued, blow up a board/home/city/country/,
triggered a nuke, started an inter galactic war, lost your job, got
elected as your country's president or became bankrupt, loose business,
something nasty, or anything I have'nt listed here,.... I do NOT take
any responsibility for either me or my employer. Touch it or it's
generated data at your own risk.

Repeat: This is NOT a tool written as an "official" software delivery
from either me or my employer. This is NOT supported in any form,
style or fashion or any official form.

If you'd like something like that.. Check out the market, if you shell
out enough $$$ you can find enough tools for that.

If you are ok with the above and still wanna help.. check out the 'How
to contribute' section.
