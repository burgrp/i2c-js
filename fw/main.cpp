int LED_PIN = 23;

class ToggleTimer : public genericTimer::Timer {

  void onTimer() {
    // target::PORT.OUTTGL.setOUTTGL(1 << LED_PIN);
    start(10);
  }
};

ToggleTimer timer;

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

void usbReset() {
  target::USB.DEVICE.EPCFG[0].reg.setEPTYPE(0, 1);
  target::USB.DEVICE.EPCFG[0].reg.setEPTYPE(1, 1);
  target::USB.DEVICE.DESCADD.setDESCADD((unsigned long)&usbDescriptor);
  target::USB.DEVICE.EPINTENSET[0]
      .reg.setRXSTP(true)
      .setTRCPT(0, true)
      .setTRCPT(1, true)
      // .setTRFAIL(0, true)
      // .setTRFAIL(1, true)
      ;
}

void interruptHandlerUSB() {
  if (target::USB.DEVICE.INTFLAG.getEORST()) {
    target::USB.DEVICE.INTFLAG.setEORST(true);
    usbReset();
  }

  for (int bank = 0; bank <= 1; bank++) {
    if (target::USB.DEVICE.EPINTFLAG[0].reg.getTRCPT(bank)) {
      target::USB.DEVICE.EPINTFLAG[0].reg.setTRCPT(bank, true);
      target::PORT.OUTCLR.setOUTCLR(11 << LED_PIN);

      // if (bank == 1) {
      //   target::USB.DEVICE.DADD = addr;
      //   usbDescriptor[5] = (unsigned int)1 << 31 | 0x3 << 28 | 0;
      //   target::USB.DEVICE.EPSTATUSSET[0].reg.setBK_RDY(1, true);
      // }
    }
  }

  // for (int bank = 0; bank <= 1; bank++) {
  //   if (target::USB.DEVICE.EPINTFLAG[0].reg.getTRFAIL(bank)) {
  //     target::USB.DEVICE.EPINTFLAG[0].reg.setTRFAIL(bank, true);
  //     target::PORT.OUTCLR.setOUTCLR(11 << LED_PIN);

  //     // target::USB.DEVICE.EPSTATUSCLR[0].reg = 0xFF;

  //   }
  // }

  if (target::USB.DEVICE.EPINTFLAG[0].reg.getRXSTP()) {
    target::USB.DEVICE.EPINTFLAG[0].reg.setRXSTP(true);

    // SET_ADDRESS
    if (buffer0.setup.bmRequestType == 0x00 && buffer0.setup.bRequest == 0x05) {
      // target::PORT.OUTTGL.setOUTTGL(11 << LED_PIN);
      target::USB.DEVICE.DADD = 0x80 | buffer0.setup.wValue;

      // usbDescriptor[5] = (unsigned int)1 << 31 | 0x3 << 28;
      // target::USB.DEVICE.EPSTATUSSET[0].reg.setBK_RDY(1, true);
    }

    // GET_DESCRIPTOR
    if (buffer0.setup.bmRequestType == 0x80 && buffer0.setup.bRequest == 0x06) {
      target::PORT.OUTTGL.setOUTTGL(11 << LED_PIN);

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

      // target::USB.DEVICE.EPSTATUSCLR[0].reg = 0xFF;
      // target::USB.DEVICE.EPSTATUSSET[0].reg = 0x80;

      // target::USB.DEVICE.EPINTFLAG[0].reg.setTRFAIL(1, true).setTRFAIL(0,
      // true);

      target::USB.DEVICE.EPSTATUSSET[0].reg.setBK_RDY(1, true);

      target::PORT.OUTSET.setOUTSET(11 << LED_PIN);

      // usbDescriptor[1] = (unsigned int)1 << 31 | 0x3 << 28 | 64;
      // target::USB.DEVICE.EPSTATUSSET[0].reg.setBK_RDY(0, true);
    }

    //usbDescriptor[1] = (unsigned int)0x3 << 28;
    target::USB.DEVICE.EPSTATUSCLR[0].reg.setBK_RDY(0, true);
  }
}

void initApplication() {

  atsamd::safeboot::init(9, false, LED_PIN);

  timer.start(10);

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

  // GC2 48MHz

  target::gclk::CLKCTRL::Register ccDpllRefClk;
  ccDpllRefClk = 0;
  ccDpllRefClk.setID(target::gclk::CLKCTRL::ID::FDPLL)
      .setGEN(target::gclk::CLKCTRL::GEN::GCLK1)
      .setCLKEN(true);

  target::GCLK.CLKCTRL = ccDpllRefClk;

  target::SYSCTRL.DPLLCTRLB.setREFCLK(target::sysctrl::DPLLCTRLB::REFCLK::GCLK);
  target::SYSCTRL.DPLLRATIO.setLDR(1490).setLDRFRAC(15);
  target::SYSCTRL.DPLLCTRLA.setENABLE(true);

  target::gclk::GENCTRL::Register gc2;
  gc2 = 0;
  gc2.setID(2)
      .setSRC(target::gclk::GENCTRL::SRC::DPLL96M)
      .setGENEN(true)
      .setOE(true); // PA16
  target::GCLK.GENCTRL = gc2;

  target::gclk::CLKCTRL::Register ccUsb;
  ccUsb = 0;
  ccUsb.setID(target::gclk::CLKCTRL::ID::USB)
      .setGEN(target::gclk::CLKCTRL::GEN::GCLK2)
      .setCLKEN(true);
  target::GCLK.CLKCTRL = ccUsb;

  target::PORT.PMUX[12].setPMUXE(target::port::PMUX::PMUXE::G);
  target::PORT.PINCFG[24].setPMUXEN(true);

  target::PORT.PMUX[12].setPMUXO(target::port::PMUX::PMUXO::G);
  target::PORT.PINCFG[25].setPMUXEN(true);

  target::PORT.PMUX[4].setPMUXE(target::port::PMUX::PMUXE::H);
  target::PORT.PINCFG[8].setPMUXEN(true);

  target::PORT.PMUX[4].setPMUXO(target::port::PMUX::PMUXO::H);
  target::PORT.PINCFG[9].setPMUXEN(true);

  target::PORT.PMUX[8].setPMUXE(target::port::PMUX::PMUXE::H);
  target::PORT.PINCFG[16].setPMUXEN(true);

  target::NVIC.ISER.setSETENA(1 << target::interrupts::External::USB);
  target::USB.DEVICE.INTENSET.setEORST(true);

  target::USB.DEVICE.CTRLB.setDETACH(false);
  target::USB.DEVICE.CTRLA.setENABLE(true);

  usbReset();
  // target::USB.DEVICE.EPINTENSET[0].reg = 1 << 4;
}
