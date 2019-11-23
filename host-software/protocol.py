class Line(object):
    pass

class StateLine(Line):
    @staticmethod
    def parse(lineParts):
        if len(lineParts) != 2:
            raise ValueError('Invalid number of parts')

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
            raise ValueError('Invalid number of parts')

        if lineParts[0] != 'POSITIONS':
            raise ValueError('Invalid first part')

        try:
            positions = int(lineParts[1])
        except ValueError:
            raise ValueError('Invalid second part')

        return ReadyLine(positions)

    def __init__(self, positions):
        self.positions = positions

    def __str__(self):
        return 'ReadyLine: %d positions' % self.positions

class CmdLine(Line):
    @staticmethod
    def parse(lineParts):
        if len(lineParts) == 0:
            raise ValueError('Invalid number of parts')
        msg = ' '.join(lineParts[1:])

        if lineParts[0] == 'OK':
            return CmdLine(True, msg)
        elif lineParts[0] == 'ERROR':
            return CmdLine(False, msg)
        else:
            raise ValueError('Invalid first part')

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
            raise ValueError('Invalid number of parts')

        msg = ' '.join(lineParts[1:])
        return MsgLine(lineParts[0], msg)

    def __init__(self, level, message):
        self.level = level
        self.message = message

    def __str__(self):
        return 'MsgLine: (%s) %s' % (self.level, self.message)
