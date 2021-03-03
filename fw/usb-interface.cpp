namespace usbd {

class UsbInterface {
public:
  UsbDevice *device;

  virtual void init() {
    for (int e = 0; UsbEndpoint *endpoint = getEndpoint(e); e++) {
      endpoint->interface = this;
      endpoint->device = device;
      endpoint->init();
    }
  }

  virtual UsbEndpoint *getEndpoint(int index) = 0;

  virtual void setup(SetupData *setupData){};
  virtual void checkDescriptor(InterfaceDescriptor *deviceDesriptor){};

  virtual const char *getLabel() = 0;
};

} // namespace usbd