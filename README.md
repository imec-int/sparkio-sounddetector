sparkio-sounddetector
=====================

Spark.io code to detect sound, also inlcudes node.js webserver to visualise the data


## Intallation

Follow the readme at [https://github.com/spark/core-firmware](https://github.com/spark/core-firmware) to flash the Spark.io using command line.

Make a symlink from our ```application.cpp``` to the  ```application.cpp``` inside the ```core-firmware/src/``` folder:

    ln -s sparkio-sounddetector/core-firmware/src/application.cpp core-firmware/src/application.cpp 

Don't forget to remove the original ```application.cpp``` first.

## Building

Go to ```core-firmware/build/``` and run ```make```.

Put your Spark in DFU-mode by holding MODE and pressing RESET once.

If the LED is flashing reddish green, flash the Spark with

    dfu-util -d 1d50:607f -a 0 -s 0x08005000:leave -D core-firmware.bin

## Debugging

To debug stuff like Arduino's and this Spark, use a serial connection. Download [CoolTerm](http://freeware.the-meiers.org/), wait for the Spark to boot up and connect to it.

Don't forget to go to __options__ to select the right serial port first.

## Turning off the Spark Cloud

If you want to turn off the Spark Cloud, change this line of ```core-firmware/src/main.cpp``` to

    SPARK_CLOUD_CONNECT = 0;