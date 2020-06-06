#!/usr/bin/python3
import logging, hwcontrol

logging.basicConfig(format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
s = hwcontrol.ShotBot('/dev/ttyUSB0')
s.ready()
s.home()
s.goto(1)
s.goto(0)
s.pump(True)
s.valve(2000)
s.pump(False)
