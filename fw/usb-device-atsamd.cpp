using namespace usbd;

namespace atsamd::usbd {

class AtSamdUsbDevice : public UsbDevice {

  int addressToSet;

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
  void init() {

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

    // GC1 -> reference of FDPLL96M

    target::gclk::CLKCTRL::Register ccDpllRefClk;
    ccDpllRefClk = 0;
    ccDpllRefClk.setID(target::gclk::CLKCTRL::ID::FDPLL).setGEN(target::gclk::CLKCTRL::GEN::GCLK1).setCLKEN(true);

    target::GCLK.CLKCTRL = ccDpllRefClk;

    // GC2 48MHz

    target::SYSCTRL.DPLLCTRLB.setREFCLK(target::sysctrl::DPLLCTRLB::REFCLK::GCLK);
    target::SYSCTRL.DPLLRATIO.setLDR(1490).setLDRFRAC(15);
    target::SYSCTRL.DPLLCTRLA.setENABLE(true);

    target::gclk::GENCTRL::Register gc2;
    gc2 = 0;
    gc2.setID(2).setSRC(target::gclk::GENCTRL::SRC::DPLL96M).setGENEN(true).setOE(true); // PA16
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

  void usbReset() {

    for (int e = 0; endpoints[e]; e++) {
      UsbEndpoint *endpoint = endpoints[e];

      for (int bank = 0; bank <= 1; bank++) {
        unsigned char *data = bank ? endpoint->txBufferPtr : endpoint->rxBufferPtr;
        int size = bank ? endpoint->txBufferSize : endpoint->rxBufferSize;

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
      if (endpoint) {
        target::USB.DEVICE.EPINTENSET[e].reg.setRXSTP(true).setTRCPT(1, true);
      }
    }
  }

  void interruptHandlerUSB() {
    if (target::USB.DEVICE.INTFLAG.getEORST()) {
      target::USB.DEVICE.INTFLAG.setEORST(true);
      usbReset();
    }

    for (int e = 0; endpoints[e]; e++) {
      UsbEndpoint *endpoint = endpoints[e];
      if (endpoint) {

        if (target::USB.DEVICE.EPINTFLAG[e].reg.getTRCPT(1)) {
          target::USB.DEVICE.EPINTFLAG[e].reg.setTRCPT(1, true);
          if (addressToSet) {
            target::USB.DEVICE.DADD = 0x80 | addressToSet;
            addressToSet = 0;
          }
        }

        if (target::USB.DEVICE.EPINTFLAG[e].reg.getRXSTP()) {
          target::USB.DEVICE.EPINTFLAG[e].reg.setRXSTP(true);
          endpoint->setup((SetupData *)endpoint->rxBufferPtr);
          target::USB.DEVICE.EPSTATUSCLR[e].reg.setBK_RDY(0, true);
        }
      }
    }
  }

  void startTx(int epIndex, int length) {
    epDescriptors[epIndex][1].PCKSIZE.MULTI_PACKET_SIZE = 0;
    epDescriptors[epIndex][1].PCKSIZE.BYTE_COUNT = length;
    target::USB.DEVICE.EPSTATUSSET[epIndex].reg.setBK_RDY(1, true);
  }

  void stall(int epIndex) { target::USB.DEVICE.EPSTATUSSET[epIndex].reg.setSTALLRQ(1, true); }

  void setAddress(int address) { addressToSet = address; };
};

} // namespace atsamd::usbd
