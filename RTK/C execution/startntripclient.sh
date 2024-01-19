#!/bin/bash
#
# $Id$
# Purpose: Start ntripclient

# change these 3 according to your needs
Stream='NAWGEO_POJ_3_1'
User='Kavin'
Password='Kavin@4236'
string='$GNGGA,103806.000,5211.6134300,N,02055.2583000,E,2,50,0.47,118.14,M,39.11,M,,*76'

DateStart=`date -u '+%s'`
SleepMin=10     # Wait min sec for next reconnect try
SleepMax=10000  # Wait max sec for next reconnect try
(while true; do
  ./ntripclient -s system.asgeupos.pl -r 8082 -m $Stream -u $User -p $Password -n $string -M 1
  if test $? -eq 0; then DateStart=`date -u '+%s'`; fi
  DateCurrent=`date -u '+%s'`
  SleepTime=`echo $DateStart $DateCurrent | awk '{printf("%d",($2-$1)*0.02)}'`
  if test $SleepTime -lt $SleepMin; then SleepTime=$SleepMin; fi
  if test $SleepTime -gt $SleepMax; then SleepTime=$SleepMax; fi
  # Sleep 2 percent of outage time before next reconnect try
  sleep $SleepTime
done) 
