import sys, time, serial

# TODO: Remove calls to sys.exit
# TODO: Use logger
class Serial(serial.Serial):
    def __init__(self, port, *args, **kwargs):
        super().__init__(port, 86400, *args, timeout=0, **kwargs)

    def __enter__(self):
        super().__enter__()
        return self

    def __exit__(self, *args):
        super().__exit__(*args)

    def readline(self):
        start = time.monotonic()
        while True:
            line = super().readline().decode('ascii').rstrip()
            if line:
                break
            time.sleep(.1)
        duration = time.monotonic() - start
        return (line, duration)

    def waitForReady(self):
        ## Wait for "INF READY POSNUM <N>" message:
        print('[hwcontrol] Waiting for the Arduino to be ready ...')
        line, dur = self.readline()
        if line[:17] != 'INF READY POSNUM ':
            print(f'ERROR: Expected "INF READY POSNUM <N>", got "{line}".')
            sys.exit(2)
        try:
            posNum = int(line[17:])
        except ValueError:
            print(f'ERROR: Expected int, got "{line[17:]}".')
            sys.exit(2)
        print('[hwcontrol] Ready after %.1fs, %d positions.' % (dur, posNum))

        return posNum

    def moveto(self, position):
        self.write(f'GOTO {position}\n'.encode('ascii'))
        print(f'[hwcontrol] Moving to position {position} ...')
        line, dur1 = self.readline()
        if line != f'CMD GOTO NONBLOCKING {position}':
            print(f'ERROR: Expected "CMD GOTO NONBLOCKING {position}", got "{line}".')
            sys.exit(2)
        line, dur2 = self.readline()
        if line != 'INF POSREACHED':
            print(f'ERROR: Expected "INF POSREACHED", got "{line}".')
            sys.exit(2)
        print('[hwcontrol] Moved to position %d in %.1fs.' % (position, dur1 + dur2))

    def home(self):
        self.write('HOME\n'.encode('ascii'))
        print('[hwcontrol] Homing ...')
        line, dur1 = self.readline()
        if line != 'CMD HOME BLOCKING':
            print(f'ERROR: Expected "CMD HOME BLOCKING", got "{line}".')
            sys.exit(2)
        line, dur2 = self.readline()
        if line != 'INF HOMED':
            print(f'ERROR: Expected "INF HOMED", got "{line}".')
            sys.exit(2)
        print('[hwcontrol] Homed in %.1fs.' % (dur1 + dur2))

    def valve(self, millis):
        self.write(f'VALVE {millis}\n'.encode('ascii'))
        print(f'[hwcontrol] Opening valve for {millis}ms ...')
        line, dur1 = self.readline()
        if line != f'CMD VALVE NONBLOCKING {millis}':
            print(f'ERROR: Expected "CMD VALVE NONBLOCKING {millis}", got "{line}".')
            sys.exit(2)
        line, dur2 = self.readline()
        if line != 'INF VALVECLOSED':
            print(f'ERROR: Expected "INF VALVECLOSED", got "{line}".')
            sys.exit(2)
        print('[hwcontrol] Opening valve done after %.1fs.' % (dur1 + dur2))

    def pump(self, enable):
        if enable:
            print('[hwcontrol] Starting pump ...')
            value = 1
        else:
            print('[hwcontrol] Stopping pump and releasing pressure ...')
            value = 0
        self.write(f'PUMP {value}\n'.encode('ascii'))
        line, dur = self.readline()
        if line != f'CMD PUMP DONE {value}':
            print(f'ERROR: Expected "CMD PUMP DONE {value}", got "{line}".')
            sys.exit(2)
        print('[hwcontrol] Switched pump in %.1fs.' % dur)
