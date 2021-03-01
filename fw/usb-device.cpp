namespace usbd {

const int MAX_ENDPOINTS = 8;

const int STD_REQUEST_GET_STATUS = 0x00;
const int STD_REQUEST_SET_ADDRESS = 0x05;
const int STD_REQUEST_GET_DESCRIPTOR = 0x06;
const int STD_REQUEST_GET_CONFIGURATION = 0x08;
const int STD_REQUEST_SET_CONFIGURATION = 0x09;

const int DESCRIPTOR_TYPE_DEVICE = 0x01;
const int DESCRIPTOR_TYPE_CONFIGURATION = 0x02;
const int DESCRIPTOR_TYPE_STRING = 0x03;
const int DESCRIPTOR_TYPE_INTERFACE = 0x04;
const int DESCRIPTOR_TYPE_ENDPOINT = 0x05;

class UsbDevice;
class UsbInterface;
class UsbEndpoint;

enum RequestRecipient { DEVICE = 0, INTERFACE = 1, ENDPOINT = 2, OTHER = 3 };
enum RequestType { STANDARD = 0, CLASS = 1, VENDOR = 2 };
enum RequestDirection { OUT = 0, IN = 1 };

enum StringDescriptorID { MANUFACTURER = 1, PRODUCT = 2, SERIAL = 3, INTERFACE_BASE = 4 };

struct __attribute__((packed)) SetupData {
  struct {
    RequestRecipient recipient : 5;
    RequestType type : 2;
    RequestDirection direction : 1;
  } bmRequestType;
  unsigned char bRequest;
  unsigned short wValue;
  unsigned short wIndex;
  unsigned short wLength;
};

struct __attribute__((packed)) DeviceDescriptor {
  unsigned char bLength;            // Length of this descriptor = 18 bytes
  unsigned char bDescriptorType;    // Descriptor type
  unsigned short bcdUSB;            // USB specification version (BCD)
  unsigned char bDeviceClass;       // Device class
  unsigned char bDeviceSubClass;    // Device subclass
  unsigned char bDeviceProtocol;    // Device Protocol
  unsigned char bMaxPacketSize0;    // Max Packet size for endpoint 0
  unsigned short idVendor;          // Vendor ID (or VID, assigned by USB-IF)
  unsigned short idProduct;         // Product ID (or PID, assigned by the manufacturer)
  unsigned short bcdDevice;         // Device release number (BCD)
  unsigned char iManufacturer;      // Index of manufacturer string
  unsigned char iProduct;           // Index of product string
  unsigned char iSerialNumber;      // Index of serial number string
  unsigned char bNumConfigurations; // Number of configurations supported
};

struct __attribute__((packed)) ConfigurationDescriptor {
  unsigned char bLength;             // Length of this descriptor = 9 bytes
  unsigned char bDescriptorType;     // Descriptor type
  unsigned short wTotalLength;       // Total length including interface and endpoint descriptors
  unsigned char bNumInterfaces;      // Number of interfaces in this configuration
  unsigned char bConfigurationValue; // Configuration value used by SET_CONFIGURATION to select this configuration
  unsigned char iConfiguration;      // Index of string that describes this configuration
  struct {
    unsigned char : 5;
    unsigned char remoteWakeup : 1;   // Remote wakeup
    unsigned char selfPowered : 1;    // Self-powered
    unsigned char reservedSetTo1 : 1; // Reserved, set to 1. (USB 1.0 Bus Powered)
  } bmAttributes;
  unsigned char bMaxPower; // Maximum power required for this configuration (in 2 mA units)
};

struct __attribute__((packed)) InterfaceDescriptor {
  unsigned char bLength;            // Length of this descriptor = 9 bytes
  unsigned char bDescriptorType;    // Descriptor type
  unsigned char bInterfaceNumber;   // Zero based index of this interface
  unsigned char bAlternateSetting;  // Alternate setting value
  unsigned char bNumEndpoints;      // Number of endpoints used by this interface (not including EP0)
  unsigned char bInterfaceClass;    // Interface class
  unsigned char bInterfaceSubclass; // Interface subclass
  unsigned char bInterfaceProtocol; // Interface protocol
  unsigned char iInterface;         // Index to string describing this interface
};

enum EndpointTransferType { CONTROL = 0, ISOCHRONOUS = 1, BULK = 2, INTERRUPT = 3 };
enum EndpointSynchronizationType { NO_SYNCHRONIZATION = 0, ASYNCHRONOUS = 1, ADAPTIVE = 2, SYNCHRONOUS = 3 };
enum EndpointUsageType { DATA = 0, FEEDBACK = 1, IMPLICIT_FEEDBACK = 2 };

struct __attribute__((packed)) EndpointDescriptor {
  unsigned char bLength;         // Length of this descriptor = 7 bytes
  unsigned char bDescriptorType; // Descriptor type
  struct {
    unsigned char index : 4; // The endpoint number
    unsigned char : 3;
    unsigned char direction : 1; // Direction. Ignored for Control, 0 = OUT endpoint, 1 = IN endpoint
  } bEndpointAddress;
  struct {
    EndpointTransferType transferType : 2;
    EndpointSynchronizationType synchronizationType : 2;
    EndpointUsageType usageType : 2;
    unsigned char : 2;
  } bmAttributes;
  unsigned short wMaxPacketSize; // Maximum packet size for this endpoint
  unsigned char bInterval;       // Polling interval in milliseconds for interrupt endpoints
};

struct __attribute__((packed)) StringDescriptor {
  unsigned char bLength;         // Length of this descriptor
  unsigned char bDescriptorType; // Descriptor type
  unsigned short unicodeData[];
};

void zeromem(void *mem, int length) {
  for (int c = 0; c < length; c++) {
    ((unsigned char *)mem)[c] = 0;
  }
}

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
  virtual void rxComplete(){};
  virtual void txComplete(){};
};

class UsbInterface {
public:
  UsbDevice *device;
  
  virtual void init() {
    for (int e = 0; UsbEndpoint* endpoint = getEndpoint(e); e++) {
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

class UsbDevice {
public:
  UsbEndpoint *endpoints[MAX_ENDPOINTS];
  int endpointCount;

  UsbControlEndpoint controlEndpoint;

  virtual void init() {
    controlEndpoint.device = this;
    controlEndpoint.init();

    for (int i = 0; UsbInterface *interface = getInterface(i); i++) {
      interface->device = this;
      interface->init();
    }

    endpointCount = 0;
    controlEndpoint.index = endpointCount++;
    endpoints[controlEndpoint.index] = &controlEndpoint;

    for (int i = 0; UsbInterface *interface = getInterface(i); i++) {
      for (int e = 0; UsbEndpoint* endpoint = interface->getEndpoint(e); e++) {
        endpoint->index = endpointCount++;
        endpoints[endpoint->index] = endpoint;
      }
    }
  }

  virtual UsbInterface *getInterface(int index) = 0;

  virtual void startTx(int epIndex, int length) = 0;
  virtual void stall(int epIndex) = 0;

  virtual void setup(SetupData *setupData){};
  virtual void checkDescriptor(DeviceDescriptor *deviceDesriptor){};

  virtual void setAddress(int address) = 0;

  virtual const char *getManufacturer() = 0;
  virtual const char *getProduct() = 0;
  virtual const char *getSerial() = 0;
};

void UsbEndpoint::startTx(int length) { device->startTx(index, length); }
void UsbEndpoint::stall() { device->stall(index); }

void UsbControlEndpoint::txComplete() {
  if (addressToSet) {
    device->setAddress(addressToSet);
    addressToSet = 0;
  }
}

void UsbControlEndpoint::setup(SetupData *setupData) {

  if (setupData->bmRequestType.recipient == DEVICE) {

    if (setupData->bmRequestType.type == STANDARD) {

      if (setupData->bRequest == STD_REQUEST_GET_DESCRIPTOR) {

        int descriptorType = setupData->wValue >> 8;
        int descriptorIndex = setupData->wValue & 0xFF;

        if (descriptorType == DESCRIPTOR_TYPE_DEVICE) {

          DeviceDescriptor *deviceDesriptor = (DeviceDescriptor *)txBufferPtr;
          zeromem(deviceDesriptor, sizeof(DeviceDescriptor));

          deviceDesriptor->bLength = sizeof(DeviceDescriptor);
          deviceDesriptor->bDescriptorType = DESCRIPTOR_TYPE_DEVICE;
          deviceDesriptor->bcdUSB = 0x200;
          deviceDesriptor->bMaxPacketSize0 = rxPacketSize;
          deviceDesriptor->iManufacturer = StringDescriptorID::MANUFACTURER;
          deviceDesriptor->iProduct = StringDescriptorID::PRODUCT;
          deviceDesriptor->iSerialNumber = StringDescriptorID::SERIAL;

          deviceDesriptor->bNumConfigurations = 1;

          device->checkDescriptor(deviceDesriptor);

          startTx(sizeof(DeviceDescriptor));

        } else if (descriptorType == DESCRIPTOR_TYPE_CONFIGURATION) {

          int totalLength = 0;

          ConfigurationDescriptor *configurationDescriptor = (ConfigurationDescriptor *)(txBuffer + totalLength);
          zeromem(configurationDescriptor, sizeof(ConfigurationDescriptor));
          configurationDescriptor->bLength = sizeof(ConfigurationDescriptor);
          configurationDescriptor->bDescriptorType = DESCRIPTOR_TYPE_CONFIGURATION;
          configurationDescriptor->bConfigurationValue = 1;
          configurationDescriptor->bmAttributes.reservedSetTo1 = 1;
          configurationDescriptor->bMaxPower = 50;

          totalLength += sizeof(ConfigurationDescriptor);

          for (int i = 0; UsbInterface *interface = device->getInterface(i); i++) {

            configurationDescriptor->bNumInterfaces++;

            InterfaceDescriptor *interfaceDescriptor = (InterfaceDescriptor *)(txBuffer + totalLength);
            zeromem(interfaceDescriptor, sizeof(InterfaceDescriptor));
            interfaceDescriptor->bLength = sizeof(InterfaceDescriptor);
            interfaceDescriptor->bDescriptorType = DESCRIPTOR_TYPE_INTERFACE;
            interfaceDescriptor->bInterfaceNumber = i;
            interfaceDescriptor->bInterfaceClass = 0xFF;
            interfaceDescriptor->iInterface = StringDescriptorID::INTERFACE_BASE + i;

            interface->checkDescriptor(interfaceDescriptor);
            totalLength += sizeof(InterfaceDescriptor);

            for (int e = 0; UsbEndpoint* endpoint = interface->getEndpoint(e); e++) {              
              for (int direction = 0; direction <= 1; direction++) {

                int packetSize = direction ? endpoint->txPacketSize : endpoint->rxPacketSize;

                if (packetSize) {

                  interfaceDescriptor->bNumEndpoints++;

                  EndpointDescriptor *endpointDescriptor = (EndpointDescriptor *)(txBuffer + totalLength);
                  zeromem(endpointDescriptor, sizeof(EndpointDescriptor));
                  endpointDescriptor->bLength = sizeof(EndpointDescriptor);
                  endpointDescriptor->bDescriptorType = DESCRIPTOR_TYPE_ENDPOINT;
                  endpointDescriptor->bEndpointAddress.direction = direction;
                  endpointDescriptor->bEndpointAddress.index = endpoint->index;
                  endpointDescriptor->bmAttributes.transferType = endpoint->transferType;
                  endpointDescriptor->bmAttributes.synchronizationType = NO_SYNCHRONIZATION;
                  endpointDescriptor->bmAttributes.usageType = DATA;
                  endpointDescriptor->wMaxPacketSize = packetSize;
                  endpointDescriptor->bInterval = endpoint->transferType == ISOCHRONOUS ? 1 : 10;

                  endpoint->checkDescriptor(endpointDescriptor);
                  totalLength += sizeof(EndpointDescriptor);
                }
              }
            }
          }

          configurationDescriptor->wTotalLength = totalLength;
          startTx(setupData->wLength < totalLength ? setupData->wLength : totalLength);

        } else if (descriptorType == DESCRIPTOR_TYPE_STRING) {

          StringDescriptor *stringDescriptor = (StringDescriptor *)txBuffer;
          stringDescriptor->bDescriptorType = DESCRIPTOR_TYPE_STRING;
          stringDescriptor->bLength = 2;

          if (descriptorIndex == 0) {

            stringDescriptor->unicodeData[0] = 0x0409;
            stringDescriptor->bLength += 2;

          } else {

            const char *str =
                descriptorIndex == StringDescriptorID::MANUFACTURER
                    ? device->getManufacturer()
                    : descriptorIndex == StringDescriptorID::PRODUCT
                          ? device->getProduct()
                          : descriptorIndex == StringDescriptorID::SERIAL
                                ? device->getSerial()
                                : descriptorIndex >= StringDescriptorID::INTERFACE_BASE &&
                                          device->getInterface(descriptorIndex - StringDescriptorID::INTERFACE_BASE)
                                      ? device->getInterface(descriptorIndex - StringDescriptorID::INTERFACE_BASE)
                                            ->getLabel()
                                      : "";
            for (int c = 0; str[c]; c++) {
              stringDescriptor->unicodeData[c] = str[c];
              stringDescriptor->bLength += 2;
            }
          }
          startTx(stringDescriptor->bLength);

        } else {
          startTx(0);
        }

      } else if (setupData->bRequest == STD_REQUEST_SET_ADDRESS) {

        addressToSet = setupData->wValue;
        startTx(0);

      } else if (setupData->bRequest == STD_REQUEST_GET_CONFIGURATION) {

        txBuffer[0] = 1;
        startTx(1);

      } else if (setupData->bRequest == STD_REQUEST_SET_CONFIGURATION) {

        startTx(0);

      } else if (setupData->bRequest == STD_REQUEST_GET_STATUS) {

        txBuffer[0] = 0;
        txBuffer[1] = 0;
        startTx(2);
      }

    } else {
      device->setup(setupData);
    }

  } else if (setupData->bmRequestType.recipient == INTERFACE) {
    UsbInterface *interface = device->getInterface(setupData->wIndex);
    if (interface) {
      interface->setup(setupData);
    }
  } else if (setupData->bmRequestType.recipient == ENDPOINT) {
    UsbEndpoint *endpoint = device->endpoints[setupData->wIndex & 0x0F];
    if (endpoint && endpoint != this) {
      endpoint->setup(setupData);
    }
  }
}

} // namespace usbd
