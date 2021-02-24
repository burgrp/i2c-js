.section .text
.weak fatalError
fatalError:
b fatalError
.section .interrupts
.weak _Z21interruptHandlerResetv
.set _Z21interruptHandlerResetv, fatalError
.word _Z21interruptHandlerResetv + 1
.weak _Z19interruptHandlerNMIv
.set _Z19interruptHandlerNMIv, fatalError
.word _Z19interruptHandlerNMIv + 1
.weak _Z25interruptHandlerHardFaultv
.set _Z25interruptHandlerHardFaultv, fatalError
.word _Z25interruptHandlerHardFaultv + 1
.word fatalError + 1
.word fatalError + 1
.word fatalError + 1
.word fatalError + 1
.word fatalError + 1
.word fatalError + 1
.word fatalError + 1
.weak _Z22interruptHandlerSVCallv
.set _Z22interruptHandlerSVCallv, fatalError
.word _Z22interruptHandlerSVCallv + 1
.word fatalError + 1
.word fatalError + 1
.weak _Z22interruptHandlerPendSVv
.set _Z22interruptHandlerPendSVv, fatalError
.word _Z22interruptHandlerPendSVv + 1
.weak _Z23interruptHandlerSysTickv
.set _Z23interruptHandlerSysTickv, fatalError
.word _Z23interruptHandlerSysTickv + 1
.weak _Z18interruptHandlerPMv
.set _Z18interruptHandlerPMv, fatalError
.word _Z18interruptHandlerPMv + 1
.weak _Z23interruptHandlerSYSCTRLv
.set _Z23interruptHandlerSYSCTRLv, fatalError
.word _Z23interruptHandlerSYSCTRLv + 1
.weak _Z19interruptHandlerWDTv
.set _Z19interruptHandlerWDTv, fatalError
.word _Z19interruptHandlerWDTv + 1
.weak _Z19interruptHandlerRTCv
.set _Z19interruptHandlerRTCv, fatalError
.word _Z19interruptHandlerRTCv + 1
.weak _Z19interruptHandlerEICv
.set _Z19interruptHandlerEICv, fatalError
.word _Z19interruptHandlerEICv + 1
.weak _Z23interruptHandlerNVMCTRLv
.set _Z23interruptHandlerNVMCTRLv, fatalError
.word _Z23interruptHandlerNVMCTRLv + 1
.weak _Z20interruptHandlerDMACv
.set _Z20interruptHandlerDMACv, fatalError
.word _Z20interruptHandlerDMACv + 1
.weak _Z19interruptHandlerUSBv
.set _Z19interruptHandlerUSBv, fatalError
.word _Z19interruptHandlerUSBv + 1
.weak _Z21interruptHandlerEVSYSv
.set _Z21interruptHandlerEVSYSv, fatalError
.word _Z21interruptHandlerEVSYSv + 1
.weak _Z23interruptHandlerSERCOM0v
.set _Z23interruptHandlerSERCOM0v, fatalError
.word _Z23interruptHandlerSERCOM0v + 1
.weak _Z23interruptHandlerSERCOM1v
.set _Z23interruptHandlerSERCOM1v, fatalError
.word _Z23interruptHandlerSERCOM1v + 1
.word fatalError + 1
.weak _Z20interruptHandlerTCC0v
.set _Z20interruptHandlerTCC0v, fatalError
.word _Z20interruptHandlerTCC0v + 1
.weak _Z19interruptHandlerTC1v
.set _Z19interruptHandlerTC1v, fatalError
.word _Z19interruptHandlerTC1v + 1
.weak _Z19interruptHandlerTC2v
.set _Z19interruptHandlerTC2v, fatalError
.word _Z19interruptHandlerTC2v + 1
.weak _Z19interruptHandlerADCv
.set _Z19interruptHandlerADCv, fatalError
.word _Z19interruptHandlerADCv + 1
.weak _Z18interruptHandlerACv
.set _Z18interruptHandlerACv, fatalError
.word _Z18interruptHandlerACv + 1
.weak _Z19interruptHandlerDACv
.set _Z19interruptHandlerDACv, fatalError
.word _Z19interruptHandlerDACv + 1