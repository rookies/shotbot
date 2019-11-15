#!/usr/bin/python3
import time, threading
import pygame

class MusicThread(threading.Thread):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        # Init pygame:
        pygame.init()
        pygame.mixer.init()
        pygame.mixer.music.set_endevent(pygame.constants.USEREVENT)

        # Init stop event:
        self.stopEvent = threading.Event()

    def __del__(self):
        # Quit mixer:
        pygame.mixer.quit()

    def run(self):
        # Play start music:
        print('[%s] Playing start music ...' % self.name)
        pygame.mixer.music.load('music_start.wav')
        pygame.mixer.music.play()

        # Loop the music:
        while not self.stopEvent.is_set():
            for event in pygame.event.get():
                if event.type == pygame.constants.USEREVENT:
                    print('[%s] Playing looping music ...' % self.name)
                    pygame.mixer.music.load('music_loop.wav')
                    pygame.mixer.music.play(-1)
            time.sleep(.1)

        # Fade out:
        print('[%s] Fading out ...' % self.name)
        pygame.mixer.music.fadeout(500)

        # Play end music:
        print('[%s] Playing end music ...' % self.name)
        pygame.mixer.music.load('music_end.wav')
        pygame.mixer.music.play()

        # Wait for end:
        pygame.event.get()
        done = False
        while not done:
            for event in pygame.event.get():
                if event.type == pygame.constants.USEREVENT:
                    done = True
            time.sleep(.1)

    def stop(self):
        self.stopEvent.set()


if __name__ == '__main__':
    t = MusicThread(name='music')
    t.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print()
        t.stop()
