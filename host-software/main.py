#!/usr/bin/python3
import sys, time, logging
import hwcontrol, music

if len(sys.argv) < 4:
    print(f'Usage: {sys.argv[0]} serialport valveTime count')
    sys.exit(1)
valveTime = int(sys.argv[2])
count = int(sys.argv[3])
assert(valveTime >= 0)
assert(count > 0)

# Setup logging:
logging.basicConfig(format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

musicThread = music.MusicThread(name='music')

def main():
    # Start music:
    musicThread.start()

    # Create hwcontrol instance:
    bot = hwcontrol.ShotBot(sys.argv[1])

    # Wait for Arduino to be ready:
    positions = bot.ready()
    if count > positions:
        logger.error('Only %d positions available, not the ordered %d',
                     positions, count)
        sys.exit(1)

    # Home:
    bot.home()

    # Move to all positions:
    try:
        bot.pump(True)
        time.sleep(2)
        # ^- TODO: Consume new messages while sleeping.
        bot.valve(valveTime)
        for p in range(1, count):
            bot.goto(p)
            bot.valve(valveTime)
    finally:
        bot.pump(False)
        musicThread.stop()

    # ... and back to zero:
    if count != 1:
        bot.goto(0)

if __name__ == '__main__':
    try:
        main()
    finally:
        musicThread.stop()

        # Wait for music to finish:
        logger.info('Waiting for music to finish ...')
        musicThread.join()
