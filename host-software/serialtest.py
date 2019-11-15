#!/usr/bin/python3
import sys, time
import hwcontrol, music

if len(sys.argv) < 4:
    print(f'Usage: {sys.argv[0]} serialport valveTime count')
    sys.exit(1)
valveTime = int(sys.argv[2])
count = int(sys.argv[3])
assert(valveTime >= 0)
assert(count > 0)

musicThread = music.MusicThread(name='music')

def main():
    ## Start music:
    musicThread.start()

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
            musicThread.stop()

        ## ... and back to zero:
        if count != 1:
            ser.moveto(0)

if __name__ == '__main__':
    try:
        main()
    finally:
        musicThread.stop()

        ## Wait for music to finish:
        print('Waiting for music to finish ...')
        musicThread.join()
