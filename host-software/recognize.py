#!/usr/bin/python3
import time, os.path
import speech_recognition as sr

def callback(recognizer, audio):
    try:
        print(recognizer.recognize_google(audio, language='en-US', show_all=True))
        #print(recognizer.recognize_sphinx(audio, grammar=os.path.join(os.path.dirname(__file__), 'grammar/command.gram')))
    except sr.UnknownValueError:
        print('Error: could not understand audio')
    except sr.RequestError as e:
        print('Error: {0}'.format(e))

r = sr.Recognizer()
m = sr.Microphone()

with m as source:
    print('Adjusting ...')
    time.sleep(1)
    r.adjust_for_ambient_noise(source)

stop_listening = r.listen_in_background(m, callback)
print('Say something!')

try:
    while True: time.sleep(5)
except KeyboardInterrupt:
    pass
