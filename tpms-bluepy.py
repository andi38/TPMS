from bluepy.btle import Scanner, DefaultDelegate

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if (isNewData or isNewDev):
          if (dev.addr.upper()[:8] == "AC:15:85"):  # my sensors MAC start with AC:15:85
            # manufacturer data = value (third field) of third AD packet
            mfdata = dev.getScanData()[2][2]
            press = (int(mfdata[6:10],16)-145)/145  # absolute pressure psi to bar (relative)
            temp = int(mfdata[4:6],16)
            batt = int(mfdata[2:4],16)/10
            print("p: ",round(press,2),"  T: ",temp,"  B: ",batt)

scanner = Scanner().withDelegate(ScanDelegate())
scanner.start()
scanner.process(60.0)
