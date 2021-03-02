int LED_PIN = 23;

/*
usb wireshark:
modprobe usbmon
sudo setfacl -m u:$USER:r /dev/usbmon*
wireshark
*/
class ToggleTimer : public genericTimer::Timer {

  void onTimer() {
    target::PORT.OUTTGL.setOUTTGL(1 << LED_PIN);
    start(10);
  }
};

ToggleTimer timer;

class CrcEndpoint : public usbd::UsbEndpoint {
public:
  unsigned char rxBuffer[1024];
  unsigned char txBuffer[8];
  void init() {
    rxBufferPtr = rxBuffer;
    rxBufferSize = sizeof(rxBuffer);
    txBufferPtr = txBuffer;
    txBufferSize = sizeof(txBuffer);
    usbd::UsbEndpoint::init();
  }
};

class TestInterface : public usbd::UsbInterface {
public:
  CrcEndpoint crcEndpoint;

  virtual UsbEndpoint* getEndpoint(int index) {
    return index == 0? &crcEndpoint: NULL;
  }
  
  const char *getLabel() { return "CRC and LED interface"; }
};

class TestDevice : public atsamd::usbd::AtSamdUsbDevice {
public:
  TestInterface testInterface;

  virtual UsbInterface* getInterface(int index) {
      return index == 0? &testInterface: NULL;
  };

  void checkDescriptor(DeviceDescriptor *deviceDesriptor) {
    deviceDesriptor->idVendor = 0xFEE0;
    deviceDesriptor->idProduct = 0x0001;
  };

  const char *getManufacturer() { return "Drake"; }
  const char *getProduct() { return "Test device"; }
};

TestDevice testDevice;

void interruptHandlerUSB() { testDevice.interruptHandlerUSB(); }

void initApplication() {

  atsamd::safeboot::init(9, false, LED_PIN);

  timer.start(10);

  testDevice.init();
}
