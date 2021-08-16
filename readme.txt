
# Application files:

main.cpp: Top level source file
controller.cpp / controller.h: These files handle low level communication to the sensor device
cmd.cpp / cmd.h: These files are for the command-line interface over the USB serial
interface.cpp / interface.h: These files handle communications over the serial connection

# Gesture Library Files:

gesture_lib.h: defines the Gesture Library API

Other gesture library source files:
	gesture.cpp
	gesture_init.cpp
	gesture_config.h
	gesture_common.h
	img_utils.cpp / img_utils.h

# Setup with mbed CLI

This project was compiled under mbed CLI.

*Installing mbed CLI*
Follow the mbed CLI installation instructions on the mbed site:
https://os.mbed.com/docs/mbed-os/v6.6/quick-start/build-with-mbed-cli.html

*Initialize a new mbed project*
Add the mbed-os library to the directory containing the source files with the following command (note the period):
  mbed new .
Use the "--scm none" option to prevent revision control initialization

*Add required libraries*
Add libraries to support the MAX32630FTHR or MAX32620FTHR development. In the same directory, type:
  mbed add https://os.mbed.com/teams/MaximIntegrated/code/max32630fthr/
or
	mbed add https://os.mbed.com/teams/MaximIntegrated/code/MAX32620FTHR/

And library to support the USB serial interface. In same directory type:
  mbed add https://os.mbed.com/teams/MaximIntegrated/code/USBDevice/


# Configuration

Stack size needs to be increased from the default of 4k.
The stack size is defined by an mbed_app.json file in the root directory of your project.

mbed_app.json should contain:

{
    "config": {
        "main-stack-size": {
            "value": 65536
        }
    }
}

# Compiling
  mbed compile -t GCC_ARM -m MAX32630FTHR
or
	mbed compile -t GCC_ARM -m MAX32620FTHR
