# syntax=docker/dockerfile:experimental

FROM ubuntu:18.04 AS base

ENV TZ=Europe/Warsaw
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

ENV DEBIAN_FRONTEND=noninteractive

RUN sed -i 's/archive.ubuntu.com/pl.archive.ubuntu.com/' /etc/apt/sources.list
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
     ubuntu-minimal debianutils build-essential bash-completion \
     libboost-all-dev libeigen3-dev \
     cmake wget \
     vim mc sharutils \
     libdouble-conversion1 libglu1-mesa libgl1 \
     libqt5gui5 libxcb-xinput0 libxcomposite1 \
     liblapacke libgfortran4 libcholmod3 libumfpack5 \
     software-properties-common apt \
     dirmngr gpg-agent \
     && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


RUN sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen && \
    locale-gen
ENV LANG=en_US.UTF-8 \
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8

RUN groupadd -g 1000 labuser \
     && useradd -u 1000 -g labuser -G video,dialout -m -d /home/labuser labuser

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
     libopengl0 \
     && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
    
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
     qtbase5-dev \
     libqt5serialport5-dev \
     libusb-1.0-0 \
     libusb-1.0-0-dev \
     libiio-dev \
     qtcreator \
     && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
    
USER labuser

WORKDIR /home/labuser


COPY --chown=labuser:labuser MT_Software_Suite_linux-x64_2022.0_b7085_r119802.tar.gz ./
RUN tar -xf MT_Software_Suite_linux-x64_2022.0_b7085_r119802.tar.gz && \
    rm -f MT_Software_Suite_linux-x64_2022.0_b7085_r119802.tar.gz && \
    tar -xf MT_Software_Suite_linux-x64_2022.0/mtmanager_linux-x64_2022.0.tar.gz && \
    rm -f MT_Software_Suite_linux-x64_2022.0/mtmanager_linux-x64_2022.0.tar.gz && \
    tar -xf MT_Software_Suite_linux-x64_2022.0/magfieldmapper_linux-x64_2022.0.tar.gz && \
    rm -f MT_Software_Suite_linux-x64_2022.0/magfieldmapper_linux-x64_2022.0.tar.gz && \
    rm -rf MT_Software_Suite_linux-x64_2022.0

ENV LD_LIBRARY_PATH=/home/labuser/mtmanager/linux-x64/bin:/home/labuser/magfieldmapper_gui/linux-x64/bin \
    PATH=/home/labuser/mtmanager/linux-x64/bin:/home/labuser/magfieldmapper_gui/linux-x64/bin:$PATH
