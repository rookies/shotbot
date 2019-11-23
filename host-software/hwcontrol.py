import sys, time, serial, enum

# TODO: Remove calls to sys.exit
# TODO: Use logger
# TODO: Allow registering good / bad responses and add callbacks?

class MessageType(enum.Enum):
    STATE = 1
    READY = 2
    CMD = 3
    MSG = 4

class Line(object):
    @staticmethod
    def parse(line):
        lineParts = line.upper().split(' ')

        if lineParts[0] == 'STATE':
            return StateLine.parse(lineParts[1:])
        elif lineParts[0] == 'READY':
            return ReadyLine.parse(lineParts[1:])
        elif lineParts[0] == 'CMD':
            return CmdLine.parse(lineParts[1:])
        elif lineParts[0] == 'MSG':
            return MsgLine.parse(lineParts[1:])
        else:
            print('Invalid line type')
            return None

class StateLine(Line):
    @staticmethod
    def parse(lineParts):
        if len(lineParts) != 2:
            print('Invalid number of parts')
            return None

        return StateLine(*lineParts)

    def __init__(self, key, value):
        self.key = key
        self.value = value

    def __str__(self):
        return 'StateLine: %s = %s' % (self.key, self.value)

class ReadyLine(Line):
    @staticmethod
    def parse(lineParts):
        if len(lineParts) != 2:
            print('Invalid number of parts')
            return None

        if lineParts[0] != 'POSITIONS':
            print('Invalid first part')
            return None

        try:
            positions = int(lineParts[1])
        except ValueError:
            print('Invalid second part')
            return None

        return ReadyLine(positions)

    def __init__(self, positions):
        self.positions = positions

    def __str__(self):
        return 'ReadyLine: %d positions' % self.positions

class CmdLine(Line):
    @staticmethod
    def parse(lineParts):
        if len(lineParts) == 0:
            print('Invalid number of parts')
            return None
        msg = ' '.join(lineParts[1:])

        if lineParts[0] == 'OK':
            return CmdLine(True, msg)
        elif lineParts[0] == 'ERROR':
            return CmdLine(False, msg)
        else:
            print('Invalid first part')
            return None

    def __init__(self, ok, message):
        self.ok = ok
        self.message = message

    def __str__(self):
        if self.ok:
            return 'CmdLine: (OK) %s' % self.message
        else:
            return 'CmdLine: (ERROR) %s' % self.message

class MsgLine(Line):
    @staticmethod
    def parse(lineParts):
        if len(lineParts) < 2:
            print('Invalid number of parts')
            return None

        msg = ' '.join(lineParts[1:])
        return MsgLine(lineParts[0], msg)

    def __init__(self, level, message):
        self.level = level
        self.message = message

    def __str__(self):
        return 'MsgLine: (%s) %s' % (self.level, self.message)

class ShotBot(object):
    def __init__(self, port):
        # Create serial port:
        self.serial = serial.Serial(port, 86400, timeout=0)

    def sendline(self, line):
        # Log it:
        print('>>> %s' % line)

        # Send it:
        self.serial.write(('%s\n' % line).encode('ascii'))

    def readline(self):
        while True:
            # Read line:
            rawLine = self.serial.readline()
            line = rawLine.decode('ascii').rstrip()
            if not line:
                continue

            # Log it:
            print('<<< %s' % line)

            # Parse it:
            parsedLine = Line.parse(line)
            if parsedLine:
                break

            # Wait for another chance:
            time.sleep(.1)

        # Return it:
        print(parsedLine)
        return parsedLine

    def ready(self):
        pass

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
