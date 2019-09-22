#!/usr/bin/python3
import sys, serial, time

if len(sys.argv) < 2:
    print(f'Usage: {sys.argv[0]} serialport')
    sys.exit(1)

def readline(ser, progressbar):
    while True:
        line = ser.readline().decode('ascii').rstrip()
        if line:
            break
        time.sleep(1)
        if progressbar:
            print('.', end='')
            sys.stdout.flush()
    if progressbar:
        print()
    return line

def moveto(ser, position):
    ser.write(f'GOTO {position}\n'.encode('ascii'))
    print(f'Moving to position {position} ', end='')
    sys.stdout.flush()
    line = readline(ser, False)
    if line != f'CMD GOTO NONBLOCKING {position}':
        print()
        print(f'ERROR: Expected "CMD GOTO NONBLOCKING {position}", got "{line}".')
        sys.exit(2)
    line = readline(ser, True)
    if line != 'INF POSREACHED':
        print()
        print(f'ERROR: Expected "INF POSREACHED", got "{line}".')
        sys.exit(2)
    print('Done.')

def home(ser):
    ser.write('HOME\n'.encode('ascii'))
    print(f'Homing ', end='')
    sys.stdout.flush()
    line = readline(ser, False)
    if line != 'CMD HOME BLOCKING':
        print()
        print(f'ERROR: Expected "CMD HOME BLOCKING", got "{line}".')
        sys.exit(2)
    line = readline(ser, True)
    if line != 'INF HOMED':
        print()
        print(f'ERROR: Expected "INF HOMED", got "{line}".')
        sys.exit(2)
    print('Done.')


with serial.Serial(sys.argv[1], 86400, timeout=0) as ser:
    ## Wait for "INF READY POSNUM <N>" message:
    print('Waiting for the Arduino to be ready ', end='')
    sys.stdout.flush()
    line = readline(ser, True)
    if line[:17] != 'INF READY POSNUM ':
        print()
        print(f'ERROR: Expected "INF READY POSNUM <N>", got "{line}".')
        sys.exit(2)
    try:
        posNum = int(line[17:])
    except ValueError:
        print()
        print(f'ERROR: Expected int, got "{line[17:]}".')
        sys.exit(2)
    print(f'Ready, {posNum} positions.')

    ## Home:
    home(ser)

    ## Move to all positions and back to zero:
    for p in range(1, posNum):
        moveto(ser, p)
    moveto(ser, 0)
