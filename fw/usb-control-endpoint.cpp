namespace usbd {

class UsbControlEndpoint : public UsbEndpoint {
  int addressToSet;

public:
  unsigned char rxBuffer[64];
  unsigned char txBuffer[64];

  void setup(SetupData *setupData);

  void init() {
    transferType = CONTROL;
    rxBufferPtr = rxBuffer;
    rxBufferSize = sizeof(rxBuffer);
    txBufferPtr = txBuffer;
    txBufferSize = sizeof(txBuffer);
    UsbEndpoint::init();
  }

  void txComplete();
};


}