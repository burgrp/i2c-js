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

class TestEndpoint: public usbd::UsbEndpoint {
public:
  unsigned char rxBuffer[16];  
  unsigned char txBuffer[64];  
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
  TestEndpoint testEndpoint;
  void init() {
    endpoints[0] = &testEndpoint;
    usbd::UsbInterface::init();
  }
};

class TestDevice : public atsamd::usbd::AtSamdUsbDevice {
public:
  TestInterface testInterface;
  void init() {
    interfaces[0] = &testInterface;
    atsamd::usbd::AtSamdUsbDevice::init();
  }

  void checkDescriptor(DeviceDescriptor *deviceDesriptor){
    deviceDesriptor->idVendor = 0xAAAA;
    deviceDesriptor->idProduct = 0xBBBB;
    };
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

  target::PORT.PMUX[8].setPMUXE(target::port::PMUX::PMUXE::H);
  target::PORT.PINCFG[16].setPMUXEN(true);

  testDevice.init();
}