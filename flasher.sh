#!/bin/sh 
avrdude -c avrispv2 -p m128 -P usb -U flash:w:main.hex
