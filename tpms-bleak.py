import asyncio
from bleak import BleakScanner
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

def found(device: BLEDevice, advertisement_data: AdvertisementData):
  if (device.address[:8] == "AC:15:85"):  # my sensors MAC start with AC:15:85
    mfdata = advertisement_data.manufacturer_data
    print(mfdata)
    for i in range(0,len(mfdata)):
      data1 = list(mfdata.keys())[i]
      list1 = [int(data1)%256,int(int(data1)/256)]
      list2 = list(mfdata[data1])
      ldata = list1 + list2
      batt = ldata[1]/10
      temp = ldata[2]
      press = ((ldata[3]*256+ldata[4])-145)/145  # absolute pressure psi to bar (relative)
      print("B: ",batt, "  T: ",temp,"  p: ",round(press,2))
#      advertisement_data={}
      device.seen_devices={}

async def main():
  scanner = BleakScanner(detection_callback=found, service_uuids=["27a5"])
  await scanner.start()
  await asyncio.sleep(300.0)
  await scanner.stop()
    
asyncio.run(main())
