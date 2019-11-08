#!/usr/bin/python3
import sys, serial, time

if len(sys.argv) < 4:
    print(f'Usage: {sys.argv[0]} serialport valveTime count')
    sys.exit(1)
valveTime = int(sys.argv[2])
count = int(sys.argv[3])
assert(valveTime >= 0)
assert(count > 0)

def readline(ser, progressbar):
    while True:
        line = ser.readline().decode('ascii').rstrip()
        if line:
            break
        time.sleep(.5)
        if progressbar:
            print('.', end='')
            sys.stdout.flush()
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
    print(' Done.')

def home(ser):
    ser.write('HOME\n'.encode('ascii'))
    print('Homing ', end='')
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
    print(' Done.')

def valve(ser, millis):
    ser.write(f'VALVE {millis}\n'.encode('ascii'))
    print(f'Opening valve for {millis}ms ', end='')
    sys.stdout.flush()
    line = readline(ser, False)
    if line != f'CMD VALVE NONBLOCKING {millis}':
        print()
        print(f'ERROR: Expected "CMD VALVE NONBLOCKING {millis}", got "{line}".')
        sys.exit(2)
    line = readline(ser, True)
    if line != 'INF VALVECLOSED':
        print()
        print(f'ERROR: Expected "INF VALVECLOSED", got "{line}".')
        sys.exit(2)
    print(' Done.')

def pump(ser, enable):
    if enable:
        print(f'Starting pump ', end='')
        value = 1
    else:
        print(f'Stopping pump ', end='')
        value = 0
    ser.write(f'PUMP {value}\n'.encode('ascii'))
    sys.stdout.flush()
    line = readline(ser, False)
    if line != f'CMD PUMP DONE {value}':
        print()
        print(f'ERROR: Expected "CMD PUMP DONE {value}", got "{line}".')
        sys.exit(2)
    print(' Done.')


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
    if count > posNum:
        print()
        print(f'ERROR: Only {posNum} positions available, not the {count} that you ordered.')
        sys.exit(1)
    print(f' Ready, {posNum} positions.')

    ## Home:
    home(ser)

    ## Move to all positions:
    try:
        pump(ser, True)
        time.sleep(1)
        valve(ser, valveTime)
        for p in range(1, count):
            moveto(ser, p)
            valve(ser, valveTime)
    finally:
        pump(ser, False)
    # ... and back to zero:
    if count != 1:
        moveto(ser, 0)
