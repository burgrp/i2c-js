int LED_PIN = 23;

enum LedMode { OFF, ON, BLINK };

LedMode ledMode = BLINK;

class ToggleTimer : public genericTimer::Timer {

  void onTimer() {
    if (ledMode == OFF) {
      target::PORT.OUTCLR.setOUTCLR(1 << LED_PIN);
    } else if (ledMode == ON) {
      target::PORT.OUTSET.setOUTSET(1 << LED_PIN);
    } else if (ledMode == BLINK) {
      target::PORT.OUTTGL.setOUTTGL(1 << LED_PIN);
    }
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

  void rxComplete(int length) {

    unsigned int crc = 0;

    for (int c = 0; c < length; c++) {
      crc += rxBuffer[c];
    }

    *(unsigned int *)&txBuffer = crc;

    startTx(4);
  }
};

class TestInterface : public usbd::UsbInterface {
public:
  CrcEndpoint crcEndpoint;

  virtual UsbEndpoint *getEndpoint(int index) { return index == 0 ? &crcEndpoint : NULL; }

  const char *getLabel() { return "CRC and LED interface"; }

  void setup(SetupData *setup) {
    if (setup->bRequest = 0x10) {
      ledMode = (LedMode)setup->wValue;
      device->getControlEndpoint()->startTx(0);
    }
  }
};

class TestDevice : public atsamd::usbd::AtSamdUsbDevice {
public:
  TestInterface testInterface;

  UsbControlEndpoint controlEndpoint;

  UsbInterface *getInterface(int index) { return index == 0 ? &testInterface : NULL; };

  UsbEndpoint *getControlEndpoint() { return &controlEndpoint; };

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

  target::PORT.PMUX[4].setPMUXE(target::port::PMUX::PMUXE::H);
  target::PORT.PINCFG[8].setPMUXEN(true);

  target::PORT.PMUX[4].setPMUXO(target::port::PMUX::PMUXO::H);
  target::PORT.PINCFG[9].setPMUXEN(true);

  testDevice.useInternalOscilators();
  testDevice.init();
}
