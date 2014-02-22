#!/bin/sh 
sudo avrdude -c jtagmkII -p m2560 -P usb -U flash:w:main.hex
