#pragma once

template<size_t L>
class CommandReader {
  public:
    CommandReader(void *(parseCommand)(char *))
      : _commandLength{0}, _parseCommand{parseCommand} {}

    void run() {
      while (Serial.available() > 0) {
        char chr = Serial.read();
        if (chr != '\n' && chr != '\r' && _commandLength < L) {
          /* Save received char in our buffer: */
          _command[_commandLength++] = chr;
        } else {
          /* Command finished, add null byte and prepare for next one: */
          _command[_commandLength] = '\0';
          _commandLength = 0;
          /* ... and parse it: */
          (*_parseCommand)(_command);
        }
      }
    }
  private:
    char _command[L+1];
    size_t _commandLength;
    void (*_parseCommand)(char *);
};
