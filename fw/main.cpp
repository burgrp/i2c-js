int LED_PIN = 23;

class ToggleTimer : public genericTimer::Timer {

  void onTimer() {
    target::PORT.OUTTGL.setOUTTGL(1 << LED_PIN);
    start(10);
  }
};

ToggleTimer timer;

void initApplication() {
  atsamd::safeboot::init(9, false, LED_PIN);

  timer.start(10);

  target::SYSCTRL.OSC8M.setPRESC(target::sysctrl::OSC8M::PRESC::_1);

  // GC0 8MHz
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

  target::USB.DEVICE.CTRLB.setDETACH(false);
  target::USB.DEVICE.CTRLA.setENABLE(true);
}
