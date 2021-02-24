#ifndef SILICON_HPP
#define SILICON_HPP

#include <stdlib.h>

namespace target {
  namespace interrupts {
    namespace Internal {
      const int Reset = 0;
      const int NMI = 1;
      const int HardFault = 2;
      const int SVCall = 10;
      const int PendSV = 13;
      const int SysTick = 14;
    }
    namespace External {
      const int PM = 0;
      const int SYSCTRL = 1;
      const int WDT = 2;
      const int RTC = 3;
      const int EIC = 4;
      const int NVMCTRL = 5;
      const int DMAC = 6;
      const int USB = 7;
      const int EVSYS = 8;
      const int SERCOM0 = 9;
      const int SERCOM1 = 10;
      const int TCC0 = 12;
      const int TC1 = 13;
      const int TC2 = 14;
      const int ADC = 15;
      const int AC = 16;
      const int DAC = 17;
    }
    namespace All {
      const int Reset = 0;
      const int NMI = 1;
      const int HardFault = 2;
      const int SVCall = 10;
      const int PendSV = 13;
      const int SysTick = 14;
      const int PM = 15;
      const int SYSCTRL = 16;
      const int WDT = 17;
      const int RTC = 18;
      const int EIC = 19;
      const int NVMCTRL = 20;
      const int DMAC = 21;
      const int USB = 22;
      const int EVSYS = 23;
      const int SERCOM0 = 24;
      const int SERCOM1 = 25;
      const int TCC0 = 27;
      const int TC1 = 28;
      const int TC2 = 29;
      const int ADC = 30;
      const int AC = 31;
      const int DAC = 32;
    }
  }
}

#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/ac.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/adc.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/dac.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/dmac.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/dsu.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/eic.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/evsys.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/gclk.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/hmatrixb.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/mtb.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/nvmctrl.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/pac.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/pm.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/port.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/rtc.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/sercom.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/sysctrl.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/tc.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/tcc.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/usb.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/wdt.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/systick.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd11c14a/generated/nvic.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-application-events/application-events.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-atsamd-safeboot/safeboot.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-generic-timer/generic-timer.cpp"
#include "/home/paul/git/i2c-js/fw/node_modules/@si14/si-systick-timer/systick-timer.cpp"
#include "/home/paul/git/i2c-js/fw/main.cpp"

#endif // SILICON_HPP