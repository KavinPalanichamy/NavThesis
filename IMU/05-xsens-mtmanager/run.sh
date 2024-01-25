#!/bin/bash -ex
docker run -it --rm \
	--volume="$HOME/.Xauthority:/home/labuser/.Xauthority:rw" \
	--volume="./imu-data-transmitter:/home/labuser/imu-data-transmitter:rw" \
	--volume="./xda_public_cpp:/home/labuser/xda_public_cpp:rw" \
	--env="DISPLAY" \
	--net=host \
	--device=/dev/ttyUSB0 \
	-p=5500 \
	xsens:latest
