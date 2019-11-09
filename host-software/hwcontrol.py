import sys, time, serial

class Serial(serial.Serial):
    def __init__(self, port, *args, **kwargs):
        super().__init__(port, 86400, *args, timeout=0, **kwargs)

    def __enter__(self):
        super().__enter__()
        return self

    def __exit__(self, *args):
        super().__exit__(*args)

    def readline(self, progressbar):
        while True:
            line = super().readline().decode('ascii').rstrip()
            if line:
                break
            time.sleep(.5)
            if progressbar:
                print('.', end='')
                sys.stdout.flush()
        return line

    def waitForReady(self):
        ## Wait for "INF READY POSNUM <N>" message:
        print('Waiting for the Arduino to be ready ', end='')
        sys.stdout.flush()
        line = self.readline(True)
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
        print(f' Ready, {posNum} positions.')

        return posNum

    def moveto(self, position):
        self.write(f'GOTO {position}\n'.encode('ascii'))
        print(f'Moving to position {position} ', end='')
        sys.stdout.flush()
        line = self.readline(False)
        if line != f'CMD GOTO NONBLOCKING {position}':
            print()
            print(f'ERROR: Expected "CMD GOTO NONBLOCKING {position}", got "{line}".')
            sys.exit(2)
        line = self.readline(True)
        if line != 'INF POSREACHED':
            print()
            print(f'ERROR: Expected "INF POSREACHED", got "{line}".')
            sys.exit(2)
        print(' Done.')

    def home(self):
        self.write('HOME\n'.encode('ascii'))
        print('Homing ', end='')
        sys.stdout.flush()
        line = self.readline(False)
        if line != 'CMD HOME BLOCKING':
            print()
            print(f'ERROR: Expected "CMD HOME BLOCKING", got "{line}".')
            sys.exit(2)
        line = self.readline(True)
        if line != 'INF HOMED':
            print()
            print(f'ERROR: Expected "INF HOMED", got "{line}".')
            sys.exit(2)
        print(' Done.')

    def valve(self, millis):
        self.write(f'VALVE {millis}\n'.encode('ascii'))
        print(f'Opening valve for {millis}ms ', end='')
        sys.stdout.flush()
        line = self.readline(False)
        if line != f'CMD VALVE NONBLOCKING {millis}':
            print()
            print(f'ERROR: Expected "CMD VALVE NONBLOCKING {millis}", got "{line}".')
            sys.exit(2)
        line = self.readline(True)
        if line != 'INF VALVECLOSED':
            print()
            print(f'ERROR: Expected "INF VALVECLOSED", got "{line}".')
            sys.exit(2)
        print(' Done.')

    def pump(self, enable):
        if enable:
            print(f'Starting pump ', end='')
            value = 1
        else:
            print(f'Stopping pump ', end='')
            value = 0
        self.write(f'PUMP {value}\n'.encode('ascii'))
        sys.stdout.flush()
        line = self.readline(False)
        if line != f'CMD PUMP DONE {value}':
            print()
            print(f'ERROR: Expected "CMD PUMP DONE {value}", got "{line}".')
            sys.exit(2)
        print(' Done.')
