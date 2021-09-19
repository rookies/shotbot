#pragma once

inline void state_set(const __FlashStringHelper *name, int value) {
  Serial.print(F("STATE "));
  Serial.print(name);
  Serial.print(F(" "));
  Serial.println(value);
}

inline void state_set(const __FlashStringHelper *name, bool value) {
  Serial.print(F("STATE "));
  Serial.print(name);
  if (value) {
    Serial.println(F(" TRUE"));
  } else {
    Serial.println(F(" FALSE"));
  }
}

inline void state_set_onoff(const __FlashStringHelper *name, bool value) {
  Serial.print(F("STATE "));
  Serial.print(name);
  if (value) {
    Serial.println(F(" ON"));
  } else {
    Serial.println(F(" OFF"));
  }
}

inline void state_set_onoff(const __FlashStringHelper *name, int value1,
                            bool value2) {
  Serial.print(F("STATE "));
  Serial.print(name);
  Serial.print(F(" "));
  Serial.print(value1);
  if (value2) {
    Serial.println(F(" ON"));
  } else {
    Serial.println(F(" OFF"));
  }
}

inline void state_set_onoff(const __FlashStringHelper *name, int value1,
                            bool value2, int value3) {
  Serial.print(F("STATE "));
  Serial.print(name);
  Serial.print(F(" "));
  Serial.print(value1);
  if (value2) {
    Serial.print(F(" ON "));
  } else {
    Serial.print(F(" OFF "));
  }
  Serial.println(value3);
}
