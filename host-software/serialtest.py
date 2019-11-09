#!/usr/bin/python3
import sys, serial, time
import hwcontrol

if len(sys.argv) < 4:
    print(f'Usage: {sys.argv[0]} serialport valveTime count')
    sys.exit(1)
valveTime = int(sys.argv[2])
count = int(sys.argv[3])
assert(valveTime >= 0)
assert(count > 0)


with hwcontrol.Serial(sys.argv[1]) as ser:
    ## Wait for Arduino to be ready:
    posNum = ser.waitForReady()
    if count > posNum:
        print()
        print(f'ERROR: Only {posNum} positions available, not the {count} that you ordered.')
        sys.exit(1)

    ## Home:
    ser.home()

    ## Move to all positions:
    try:
        ser.pump(True)
        time.sleep(1)
        ser.valve(valveTime)
        for p in range(1, count):
            ser.moveto(p)
            ser.valve(valveTime)
    finally:
        ser.pump(False)

    ## ... and back to zero:
    if count != 1:
        ser.moveto(0)
