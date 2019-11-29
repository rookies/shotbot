#!/usr/bin/python3
import sys, time
import hwcontrol

if len(sys.argv) < 2:
    print(f'Usage: {sys.argv[0]} serialport')
    sys.exit(1)


bot = hwcontrol.ShotBot(sys.argv[1])
bot.ready()
bot.valve(10000)
