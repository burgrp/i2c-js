namespace usbd {

class UsbEndpoint {
public:
  UsbInterface *interface;
  UsbDevice *device;
  EndpointTransferType transferType = INTERRUPT;
  unsigned char *rxBufferPtr;
  unsigned int rxBufferSize;
  unsigned int rxPacketSize;
  unsigned char *txBufferPtr;
  unsigned int txBufferSize;
  unsigned int txPacketSize;
  unsigned int index;
  virtual void init() {
    rxPacketSize = rxBufferSize > 64 ? 64 : rxBufferSize;
    txPacketSize = txBufferSize > 64 ? 64 : txBufferSize;
  }
  virtual void setup(SetupData *setupData) {}
  virtual void checkDescriptor(EndpointDescriptor *deviceDesriptor){};
  void startTx(int length);
  void stall();
  virtual void rxComplete(int length){};
  virtual void txComplete(){};
};

} // namespace usbd