#!/bin/sh 
sudo avrdude -c avrispv2 -p m88 -P usb -U flash:w:main.hex
