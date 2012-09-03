#!/bin/sh 
avrdude -c avrispv2 -p $1 -P usb -U flash:w:main.hex
