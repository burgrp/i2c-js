using namespace usbd;

namespace atsamd::usbd {

const int SERIAL_NUMBER_LENGTH = 16;

class AtSamdUsbDevice : public UsbDevice {

  struct {
    unsigned char *ADDR;
    struct {
      unsigned int BYTE_COUNT : 14;
      unsigned int MULTI_PACKET_SIZE : 14;
      unsigned int SIZE : 3;
      unsigned int AUTO_ZLP : 1;
    } PCKSIZE;
    struct {
      unsigned int SUBPID : 4;
      unsigned int VARIABLE : 11;
    } EXTREG;
    struct {
      unsigned int CRCERR : 1;
      unsigned int ERROFLOW : 1;
    } STATUS_BK;
  } epDescriptors[8][2];

public:
  unsigned char serialNumber[SERIAL_NUMBER_LENGTH + 1];

  void init() {

    // calculate serial number

    unsigned int snWordAddresses[4] = {0x0080A00C, 0x0080A040, 0x0080A044, 0x0080A048};

    for (int i = 0; i < SERIAL_NUMBER_LENGTH; i++) {
      unsigned int word = *(unsigned int *)(snWordAddresses[i >> 2]);
      unsigned char byte = word >> (i & 3);
      unsigned char v = (byte >> 4) + (byte & 0xF);
      serialNumber[i] = v < 10 ? v + '0' : v - 10 + 'A';
    }
    serialNumber[sizeof(serialNumber) - 1] = 0;

    UsbDevice::init();

    // GC0 8MHz

    target::SYSCTRL.OSC8M.setPRESC(target::sysctrl::OSC8M::PRESC::_1);

    target::gclk::GENCTRL::Register gc0;
    gc0 = 0;
    gc0.setID(0).setSRC(target::gclk::GENCTRL::SRC::OSC8M).setGENEN(true).setOE(true); // PA8
    target::GCLK.GENCTRL = gc0;

    // GC1 32kHz

    target::SYSCTRL.OSC32K.setENABLE(true).setEN32K(true);

    target::gclk::GENCTRL::Register gc1;
    gc1 = 0;
    gc1.setID(1).setSRC(target::gclk::GENCTRL::SRC::OSC32K).setGENEN(true).setOE(true); // PA9
    target::GCLK.GENCTRL = gc1;

    // GC1 -> reference of DFLL48

    target::gclk::CLKCTRL::Register ccDpllRefClk;
    ccDpllRefClk = 0;
    ccDpllRefClk.setGEN(target::gclk::CLKCTRL::GEN::GCLK1).setID(target::gclk::CLKCTRL::ID::DFLL48).setCLKEN(true);
    target::GCLK.CLKCTRL = ccDpllRefClk;

    // GC2 48MHz

    target::sysctrl::DFLLCTRL::Register dfllCtrl;
    dfllCtrl = 0;
    dfllCtrl.setUSBCRM(true).setQLDIS(false).setCCDIS(true).setENABLE(true);
    target::SYSCTRL.DFLLCTRL = dfllCtrl;

    //for (volatile int c = 0; c < 100000; c++);

    target::SYSCTRL.DFLLVAL.setCOARSE(102);
    target::SYSCTRL.DFLLVAL.setFINE(23);

    for (volatile int c = 0; c < 100000; c++);

    target::gclk::GENCTRL::Register gc2;
    gc2 = 0;
    gc2.setID(2).setSRC(target::gclk::GENCTRL::SRC::DFLL48M).setGENEN(true).setOE(true); // PA16
    target::GCLK.GENCTRL = gc2;

    // GC2 -> USB

    target::gclk::CLKCTRL::Register ccUsb;
    ccUsb = 0;
    ccUsb.setID(target::gclk::CLKCTRL::ID::USB).setGEN(target::gclk::CLKCTRL::GEN::GCLK2).setCLKEN(true);
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

  void setAddress(int address) { target::USB.DEVICE.DADD = 0x80 | address; }

  void usbReset() {

    for (int e = 0; endpoints[e]; e++) {
      UsbEndpoint *endpoint = endpoints[e];

      for (int bank = 0; bank <= 1; bank++) {
        unsigned char *data = bank ? endpoint->txBufferPtr : endpoint->rxBufferPtr;
        int size = bank ? endpoint->txPacketSize : endpoint->rxPacketSize;

        if (endpoint && data && size) {
          target::USB.DEVICE.EPCFG[e].reg.setEPTYPE(bank, 1 + endpoint->transferType);
          epDescriptors[e][bank].ADDR = data;
          epDescriptors[e][bank].PCKSIZE.SIZE =
              size == 0
                  ? 0
                  : size == 16
                        ? 1
                        : size == 32
                              ? 2
                              : size == 64 ? 3
                                           : size == 128 ? 4 : size == 256 ? 5 : size == 512 ? 6 : size == 1023 ? 7 : 0;
          epDescriptors[e][bank].PCKSIZE.AUTO_ZLP = 1;

        } else {
          target::USB.DEVICE.EPCFG[e].reg.setEPTYPE(bank, 0);
        }
      }
    }

    target::USB.DEVICE.DESCADD.setDESCADD((unsigned long)&epDescriptors);

    for (int e = 0; endpoints[e]; e++) {
      UsbEndpoint *endpoint = endpoints[e];
      target::USB.DEVICE.EPINTENSET[e].reg.setRXSTP(true).setTRCPT(0, true).setTRCPT(1, true);
    }
  }

  void interruptHandlerUSB() {
    if (target::USB.DEVICE.INTFLAG.getEORST()) {
      target::USB.DEVICE.INTFLAG.setEORST(true);
      usbReset();
    }

    for (int e = 0; endpoints[e]; e++) {
      UsbEndpoint *endpoint = endpoints[e];

      if (target::USB.DEVICE.EPINTFLAG[e].reg.getTRCPT(0)) {
        endpoint->rxComplete();
        target::USB.DEVICE.EPINTFLAG[e].reg.setTRCPT(0, true);
        epDescriptors[e][0].PCKSIZE.BYTE_COUNT = 0;
        target::USB.DEVICE.EPSTATUSCLR[e].reg.setBK_RDY(0, true);
      }

      if (target::USB.DEVICE.EPINTFLAG[e].reg.getTRCPT(1)) {
        endpoint->txComplete();
        target::USB.DEVICE.EPINTFLAG[e].reg.setTRCPT(1, true);
      }

      if (target::USB.DEVICE.EPINTFLAG[e].reg.getRXSTP()) {
        endpoint->setup((SetupData *)endpoint->rxBufferPtr);
        target::USB.DEVICE.EPINTFLAG[e].reg.setRXSTP(true);
        epDescriptors[e][0].PCKSIZE.BYTE_COUNT = 0;
        target::USB.DEVICE.EPSTATUSCLR[e].reg.setBK_RDY(0, true);
      }
    }
  }

  void startTx(int epIndex, int length) {
    epDescriptors[epIndex][1].PCKSIZE.MULTI_PACKET_SIZE = 0;
    epDescriptors[epIndex][1].PCKSIZE.BYTE_COUNT = length;
    target::USB.DEVICE.EPSTATUSSET[epIndex].reg.setBK_RDY(1, true);
  }

  void stall(int epIndex) { target::USB.DEVICE.EPSTATUSSET[epIndex].reg.setSTALLRQ(1, true); }

  const char *getSerial() { return (const char *)&serialNumber; }
};

} // namespace atsamd::usbd
