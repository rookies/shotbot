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
        self._serial = serial.Serial(port, 86400, timeout=1)

        # Create hardware state:
        self._state = {}

    def _sendline(self, line):
        # Log it:
        print('>>> %s' % line)

        # Send it:
        self._serial.write(('%s\n' % line).encode('ascii'))

    # TODO: Add timeout
    def _readline(self):
        while True:
            # Read line:
            rawLine = self._serial.readline()
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

        # If it's a state line, save it to our state:
        if isinstance(parsedLine, StateLine):
            self._state[parsedLine.key] = parsedLine.value
            print('New state:', self._state)

        # If it's a message line, print it to the console:
        if isinstance(parsedLine, MsgLine):
            print(parsedLine)

        # Return it:
        return parsedLine

    # TODO: Add timeout
    def _waitForState(self, key, value):
        while True:
            self._readline()
            if (key in self._state) and (self._state[key] == value):
                return True

    def ready(self):
        print('[hwcontrol] Waiting for the Arduino to be ready ...')
        while True:
            line = self._readline()
            if isinstance(line, ReadyLine):
                print('[hwcontrol] Ready, %d positions.' % line.positions)
                return line.positions

    # TODO: Handle errors
    def home(self):
        print('[hwcontrol] Homing ...')
        self._sendline('HOME')
        self._waitForState('HOMED', 'TRUE')
        print('[hwcontrol] Homed.')

    # TODO: Handle errors
    def goto(self, position):
        print('[hwcontrol] Moving to position %d ...' % position)
        self._sendline('GOTO %d' % position)
        self._waitForState('POSITION', str(position))
        print('[hwcontrol] Moved to position %d' % position)

    # TODO: Handle errors
    def valve(self, millis):
        print(f'[hwcontrol] Opening valve for {millis}ms ...')
        self._sendline('VALVE %d' % millis)
        if millis != 0:
            self._waitForState('VALVE', 'OPEN')
        self._waitForState('VALVE', 'CLOSED')
        print('[hwcontrol] Opening valve done.')

    # TODO: Handle errors
    def pump(self, enable):
        if enable:
            print('[hwcontrol] Starting pump ...')
            self._sendline('PUMP 1')
            self._waitForState('PUMP', 'ON')
            print('[hwcontrol] Started pump.')
        else:
            print('[hwcontrol] Stopping pump and releasing pressure ...')
            self._sendline('PUMP 0')
            self._waitForState('PUMP', 'OFF')
            print('[hwcontrol] Stopped pump.')
