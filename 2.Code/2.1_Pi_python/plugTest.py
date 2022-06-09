import asyncio
import time
from kasa import SmartPlug

async def main():
    p = SmartPlug("192.168.4.44")

    await p.update()  # Request the update
    print(p.alias)  # Print out the alias
    print(p.is_on)
    print(p.is_off)
    print(p.emeter_realtime)  # Print out current emeter status

    await p.turn_off()  # Turn the device off
    time.sleep(2.0)

if __name__ == "__main__":
    asyncio.run(main())