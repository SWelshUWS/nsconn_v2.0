#!/bin/bash

# script to install nsconn
# run me as superuser

sudo apt install libbluetooth-dev libglib2.0-dev libjson-c-dev

CONF=/etc/.nsconn.conf
if [ ! -f "$CONF" ];
then
	sudo cp ../config/nsconn.conf /etc/	
else
	echo "$CONF already exists."
fi

REC=/usr/local/share/nsconn/record
if [ ! -d "$REC" ];
then
  sudo mkdir -p /usr/local/share/nsconn/record
else
	echo "$REC already exists."
fi

NSMAN=/usr/share/man/man1/nsconn.1
if [ ! -f "$NSMAN" ];
then
  echo "created man entry for nsconn."
  sudo cp ../docs/nsconn.1 /usr/share/man/man1/
else
	echo "$NSMAN already exists."
fi

gcc -o /usr/bin/nsconn nsconn.c parsens.c format.c forward.c outsock.c btsock.c -lbluetooth -ljson-c `pkg-config --cflags --libs glib-2.0`