#!/usr/bin/python3
import time
import pygame

# Init pygame:
pygame.init()
pygame.mixer.init()
pygame.mixer.music.set_endevent(pygame.constants.USEREVENT)

# Play start music:
print('Playing start music ...')
pygame.mixer.music.load('music_start.wav')
pygame.mixer.music.play()

# Main loop:
try:
    while True:
        for event in pygame.event.get():
            if event.type == pygame.constants.USEREVENT:
                print('Playing looping music ...')
                pygame.mixer.music.load('music_loop.wav')
                pygame.mixer.music.play(-1)
        time.sleep(.1)
except KeyboardInterrupt:
    print('Fading out ...')
    pygame.mixer.music.fadeout(500)
    print('Playing end music ...')
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

# Quit mixer:
pygame.mixer.quit()
