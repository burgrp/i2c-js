namespace usbd {

const int MAX_ENDPOINTS = 8;
const int MAX_INTERFACES = 8;
const int MAX_CONFIGURATIONS = 8;

class UsbDevice;
class UsbConfiguration;
class UsbInterface;
class UsbEndpoint;

class UsbEndpoint {
public:
  UsbInterface *interface;

  virtual void init() {}
};

class UsbInterface {
public:
  UsbConfiguration *configuration;
  UsbEndpoint *endpoints[MAX_ENDPOINTS];

  virtual void init() {
    for (int c = 0; endpoints[c]; c++) {
      endpoints[c]->interface = this;
      endpoints[c]->init();
    }
  }
};

class UsbConfiguration {
public:
  UsbDevice *device;
  UsbInterface *interfaces[MAX_INTERFACES];

  virtual void init() {
    for (int c = 0; interfaces[c]; c++) {
      interfaces[c]->configuration = this;
      interfaces[c]->init();
    }
  }
};

class ControlEndpoint : public UsbEndpoint {};

class UsbDevice {
public:
  UsbConfiguration *configurations[MAX_CONFIGURATIONS];

  int currentConfiguration;
  ControlEndpoint controlEndpoint;

  UsbEndpoint *endpoints[MAX_ENDPOINTS];
  int endpointCount;

  void selectConfiguration(int index) {

    currentConfiguration = index;
    endpointCount = 0;

    endpoints[endpointCount++] = &controlEndpoint;

    if (configurations[currentConfiguration]) {
      UsbConfiguration *configuration = configurations[currentConfiguration];
      for (int i = 0; configuration->interfaces[i]; i++) {
        UsbInterface *interface = configuration->interfaces[i];
        for (int e = 0; interface->endpoints[e]; e++) {
          endpoints[endpointCount++] = interface->endpoints[e];
        }
      }
    }
  }

  virtual void init() {
    for (int c = 0; configurations[c]; c++) {
      configurations[c]->device = this;
      configurations[c]->init();
    }
    selectConfiguration(0);
  }
};

} // namespace usbd

using namespace usbd;

namespace atsamd::usbd {

union {
  struct {
    unsigned char bmRequestType;
    unsigned char bRequest;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
  } setup;
  unsigned char raw[64];
} buffer0;
unsigned char buffer1[64];

unsigned int usbDescriptor[8] = {(unsigned int)&buffer0,
                                 (unsigned int)0x3 << 28, // 64 bytes max size
                                 0,
                                 0,
                                 (unsigned int)&buffer1,
                                 (unsigned int)1 << 31 |
                                     0x3 << 28, // Auto-ZLP, 64 bytes max size
                                 0,
                                 0};

class AtSamdUsbDevice : public UsbDevice {

  int addressToSet;

public:
  void init() {

    UsbDevice::init();

    // GC0 8MHz

    target::SYSCTRL.OSC8M.setPRESC(target::sysctrl::OSC8M::PRESC::_1);

    target::gclk::GENCTRL::Register gc0;
    gc0 = 0;
    gc0.setID(0)
        .setSRC(target::gclk::GENCTRL::SRC::OSC8M)
        .setGENEN(true)
        .setOE(true); // PA8
    target::GCLK.GENCTRL = gc0;

    // GC1 32kHz

    target::SYSCTRL.OSC32K.setENABLE(true).setEN32K(true);

    target::gclk::GENCTRL::Register gc1;
    gc1 = 0;
    gc1.setID(1)
        .setSRC(target::gclk::GENCTRL::SRC::OSC32K)
        .setGENEN(true)
        .setOE(true); // PA9
    target::GCLK.GENCTRL = gc1;

    // GC1 -> reference of FDPLL96M

    target::gclk::CLKCTRL::Register ccDpllRefClk;
    ccDpllRefClk = 0;
    ccDpllRefClk.setID(target::gclk::CLKCTRL::ID::FDPLL)
        .setGEN(target::gclk::CLKCTRL::GEN::GCLK1)
        .setCLKEN(true);

    target::GCLK.CLKCTRL = ccDpllRefClk;

    // GC2 48MHz

    target::SYSCTRL.DPLLCTRLB.setREFCLK(
        target::sysctrl::DPLLCTRLB::REFCLK::GCLK);
    target::SYSCTRL.DPLLRATIO.setLDR(1490).setLDRFRAC(15);
    target::SYSCTRL.DPLLCTRLA.setENABLE(true);

    target::gclk::GENCTRL::Register gc2;
    gc2 = 0;
    gc2.setID(2)
        .setSRC(target::gclk::GENCTRL::SRC::DPLL96M)
        .setGENEN(true)
        .setOE(true); // PA16
    target::GCLK.GENCTRL = gc2;

    // GC2 -> USB

    target::gclk::CLKCTRL::Register ccUsb;
    ccUsb = 0;
    ccUsb.setID(target::gclk::CLKCTRL::ID::USB)
        .setGEN(target::gclk::CLKCTRL::GEN::GCLK2)
        .setCLKEN(true);
    target::GCLK.CLKCTRL = ccUsb;

    // enable USB pins

    target::PORT.PMUX[12].setPMUXE(target::port::PMUX::PMUXE::G);
    target::PORT.PINCFG[24].setPMUXEN(true);

    target::PORT.PMUX[12].setPMUXO(target::port::PMUX::PMUXO::G);
    target::PORT.PINCFG[25].setPMUXEN(true);

    // enable interrupts
    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::USB);
    target::USB.DEVICE.INTENSET.setEORST(true);

    // attach
    target::USB.DEVICE.CTRLB.setDETACH(false);
    target::USB.DEVICE.CTRLA.setENABLE(true);

    usbReset();

  }

  void usbReset() {
    target::USB.DEVICE.EPCFG[0].reg.setEPTYPE(0, 1);
    target::USB.DEVICE.EPCFG[0].reg.setEPTYPE(1, 1);
    target::USB.DEVICE.DESCADD.setDESCADD((unsigned long)&usbDescriptor);
    target::USB.DEVICE.EPINTENSET[0]
        .reg
        .setRXSTP(true)
        //.setTRCPT(0, true)
        .setTRCPT(1, true);
  }

  void interruptHandlerUSB() {
    if (target::USB.DEVICE.INTFLAG.getEORST()) {
      target::USB.DEVICE.INTFLAG.setEORST(true);
      usbReset();
    }

    if (target::USB.DEVICE.EPINTFLAG[0].reg.getTRCPT(1)) {
      target::USB.DEVICE.EPINTFLAG[0].reg.setTRCPT(1, true);
      if (addressToSet) {
        target::USB.DEVICE.DADD = addressToSet;
        addressToSet = 0;
      }
    }

    if (target::USB.DEVICE.EPINTFLAG[0].reg.getRXSTP()) {
      target::USB.DEVICE.EPINTFLAG[0].reg.setRXSTP(true);

      // SET_ADDRESS
      if (buffer0.setup.bmRequestType == 0x00 &&
          buffer0.setup.bRequest == 0x05) {

        usbDescriptor[5] = (unsigned int)1 << 31 | 0x3 << 28;
        target::USB.DEVICE.EPSTATUSSET[0].reg.setBK_RDY(1, true);

        addressToSet = 0x80 | buffer0.setup.wValue;
      }

      // GET_DESCRIPTOR
      if (buffer0.setup.bmRequestType == 0x80 &&
          buffer0.setup.bRequest == 0x06) {
        // target::PORT.OUTTGL.setOUTTGL(11 << LED_PIN);

        buffer1[0] = 18;
        buffer1[1] = 0x01;
        buffer1[2] = 0;
        buffer1[3] = 2;
        buffer1[4] = 0xFF;
        buffer1[7] = 64;
        buffer1[8] = 0x11;
        buffer1[10] = 0x12;
        buffer1[17] = 1;

        usbDescriptor[5] = (unsigned int)1 << 31 | 0x3 << 28 | 18;

        target::USB.DEVICE.EPSTATUSSET[0].reg.setBK_RDY(1, true);
      }

      target::USB.DEVICE.EPSTATUSCLR[0].reg.setBK_RDY(0, true);
    }
  }
};

} // namespace atsamd::usbd
