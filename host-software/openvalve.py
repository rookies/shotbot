#!/usr/bin/python3
import sys, time
import hwcontrol

if len(sys.argv) < 2:
    print(f'Usage: {sys.argv[0]} serialport')
    sys.exit(1)


with hwcontrol.Serial(sys.argv[1]) as ser:
    ## Wait for Arduino to be ready:
    ser.waitForReady()

    ## Open valve:
    ser.valve(10000)
