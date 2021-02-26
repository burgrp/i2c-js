namespace usbd {

const int MAX_ENDPOINTS = 8;
const int MAX_INTERFACES = 8;

enum TransferType { CONTROL = 0, ISOCHRONOUS = 1, BULK = 2, INTERRUPT = 3 };

class UsbDevice;
class UsbInterface;
class UsbEndpoint;

struct SetupData {
  unsigned char bmRequestType;
  unsigned char bRequest;
  unsigned short wValue;
  unsigned short wIndex;
  unsigned short wLength;
};

class UsbEndpoint {
public:
  UsbInterface *interface;
  UsbDevice *device;
  TransferType transferType = CONTROL;
  unsigned char *rxBufferPtr;
  unsigned int rxBufferSize;
  unsigned char *txBufferPtr;
  unsigned int txBufferSize;
  unsigned int index;
  virtual void init() {}
  virtual void setup(SetupData *setupData) {}
  void startTx(int length);
};

class UsbInterface {
public:
  UsbDevice *device;
  UsbEndpoint *endpoints[MAX_ENDPOINTS];

  virtual void init() {
    for (int c = 0; endpoints[c]; c++) {
      endpoints[c]->interface = this;
      endpoints[c]->device = device;
      endpoints[c]->init();
    }
  }
};

class ControlEndpoint : public UsbEndpoint {
public:
  unsigned char rxBuffer[64];
  unsigned char txBuffer[64];

  void setDeviceAddress(int address);

  void setup(SetupData *setupData) {

    // GET_DESCRIPTOR
    if (setupData->bmRequestType == 0x80 && setupData->bRequest == 0x06) {

      txBuffer[0] = 18;
      txBuffer[1] = 0x01;
      txBuffer[2] = 0;
      txBuffer[3] = 2;
      txBuffer[4] = 0xFF;
      txBuffer[7] = 64;
      txBuffer[8] = 0x11;
      txBuffer[10] = 0x12;
      txBuffer[17] = 1;

      startTx(18);
    }

    // SET_ADDRESS
    if (setupData->bmRequestType == 0x00 && setupData->bRequest == 0x05) {
      setDeviceAddress(setupData->wValue);
      startTx(0);
    }
  }

  void init() {
    rxBufferPtr = rxBuffer;
    rxBufferSize = sizeof(rxBuffer);
    txBufferPtr = txBuffer;
    txBufferSize = sizeof(txBuffer);
    UsbEndpoint::init();
  }
};

class UsbDevice {
public:
  UsbInterface *interfaces[MAX_INTERFACES];

  UsbEndpoint *endpoints[MAX_ENDPOINTS];
  int endpointCount;

  ControlEndpoint controlEndpoint;

  virtual void init() {

    controlEndpoint.device = this;
    controlEndpoint.init();

    for (int i = 0; interfaces[i]; i++) {
      interfaces[i]->device = this;
      interfaces[i]->init();
    }

    endpointCount = 0;
    controlEndpoint.index = endpointCount++;
    endpoints[controlEndpoint.index] = &controlEndpoint;

    for (int i = 0; interfaces[i]; i++) {
      UsbInterface *interface = interfaces[i];
      for (int e = 0; interface->endpoints[e]; e++) {
        UsbEndpoint* endpoint = interface->endpoints[e];
        endpoint->index = endpointCount++;
        endpoints[endpoint->index] = endpoint;
      }
    }
  }

  virtual void startTx(int epIndex, int length) = 0;
  virtual void setAddress(int address) = 0;
};

void UsbEndpoint::startTx(int length) {
  device->startTx(index, length);
}

void ControlEndpoint::setDeviceAddress(int address) {
  device->setAddress(address);
}

} // namespace usbd

