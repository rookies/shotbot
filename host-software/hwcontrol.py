import sys, time, serial, enum, logging
from protocol import *

# TODO: Allow registering good / bad responses and add callbacks?

class ShotBot(object):
    def __init__(self, port, debug=False):
        # Create serial port:
        self._serial = serial.Serial(port, 86400, timeout=1)

        # Create hardware state:
        self._state = {}

        # Create logger:
        self._logger = logging.getLogger(__name__)
        if debug:
            self._logger.setLevel(logging.DEBUG)
        else:
            self._logger.setLevel(logging.INFO)

    def _sendline(self, line):
        self._logger.debug('Sending line: %s', line)
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
            self._logger.debug('Received line: %s', line)

            # Parse it:
            parsedLine = None
            try:
                lineParts = line.upper().split(' ')

                if lineParts[0] == 'STATE':
                    parsedLine = StateLine.parse(lineParts[1:])
                elif lineParts[0] == 'READY':
                    parsedLine = ReadyLine.parse(lineParts[1:])
                elif lineParts[0] == 'CMD':
                    parsedLine = CmdLine.parse(lineParts[1:])
                elif lineParts[0] == 'MSG':
                    parsedLine = MsgLine.parse(lineParts[1:])
                else:
                    raise ValueError('Invalid line type')
            except ValueError as err:
                self._logger.error(err)

            # Check it:
            if parsedLine:
                break

            # Wait for another chance:
            time.sleep(.1)

        # If it's a state line, save it to our state:
        if isinstance(parsedLine, StateLine):
            self._state[parsedLine.key] = parsedLine.value
            # TODO: Fancier state output
            print('New state:', self._state)

        # If it's a message line, print it to the console:
        if isinstance(parsedLine, MsgLine):
            # TODO: Output via logger
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
        self._logger.info('Waiting for the Arduino to be ready ...')
        while True:
            line = self._readline()
            if isinstance(line, ReadyLine):
                self._logger.info('Ready, %d positions', line.positions)
                return line.positions

    # TODO: Handle errors
    def home(self):
        self._logger.info('Homing ...')
        self._sendline('HOME')
        self._waitForState('HOMED', 'TRUE')
        self._logger.info('Homed')

    # TODO: Handle errors
    def goto(self, position):
        self._logger.info('Moving to position %d ...', position)
        self._sendline('GOTO %d' % position)
        self._waitForState('POSITION', str(position))
        self._logger.info('Moved to position %d', position)

    # TODO: Handle errors
    def valve(self, millis):
        self._logger.info('Opening valve for %dms ...', millis)
        self._sendline('VALVE %d' % millis)
        if millis != 0:
            self._waitForState('VALVE', 'OPEN')
        self._waitForState('VALVE', 'CLOSED')
        self._logger.info('Opening valve done')

    # TODO: Handle errors
    def pump(self, enable):
        if enable:
            self._logger.info('Starting pump ...')
            self._sendline('PUMP 1')
            self._waitForState('PUMP', 'ON')
            self._logger.info('Started pump')
        else:
            self._logger.info('Stopping pump and releasing pressure ...')
            self._sendline('PUMP 0')
            self._waitForState('PUMP', 'OFF')
            self._logger.info('Stopped pump')
