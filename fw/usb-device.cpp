#include <cstring>

namespace usbd {

const int MAX_ENDPOINTS = 8;
const int MAX_INTERFACES = 8;

const int DEVICE_REQUEST_SET_ADDRESS = 0x05;
const int DEVICE_REQUEST_GET_DESCRIPTOR = 0x06;
const int DEVICE_REQUEST_GET_CONFIGURATION = 0x08;
const int DEVICE_REQUEST_SET_CONFIGURATION = 0x09;

const int DESCRIPTOR_TYPE_DEVICE = 1;
const int DESCRIPTOR_TYPE_CONFIGURATION = 2;
const int DESCRIPTOR_TYPE_INTERFACE = 0x04;
const int DESCRIPTOR_TYPE_ENDPOINT = 0x05;

enum TransferType { CONTROL = 0, ISOCHRONOUS = 1, BULK = 2, INTERRUPT = 3 };
enum SynchronizationType { NO_SYNCHRONIZATION = 0, ASYNCHRONOUS = 1, ADAPTIVE = 2, SYNCHRONOUS = 3 };
enum UsageType { DATA = 0, FEEDBACK = 1, IMPLICIT_FEEDBACK = 2 };

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

struct __attribute__((packed)) DeviceDescriptor {
  unsigned char bLength;            // Length of this descriptor = 18 bytes
  unsigned char bDescriptorType;    // Descriptor type = DEVICE (01h)
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
  unsigned char bDescriptorType;     // Descriptor type = CONFIGURATION (02h)
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
  unsigned char bDescriptorType;    // Descriptor type = INTERFACE (04h)
  unsigned char bInterfaceNumber;   // Zero based index of this interface
  unsigned char bAlternateSetting;  // Alternate setting value
  unsigned char bNumEndpoints;      // Number of endpoints used by this interface (not including EP0)
  unsigned char bInterfaceClass;    // Interface class
  unsigned char bInterfaceSubclass; // Interface subclass
  unsigned char bInterfaceProtocol; // Interface protocol
  unsigned char iInterface;         // Index to string describing this interface
};

struct __attribute__((packed)) EndpointDescriptor {
  unsigned char bLength;         // Length of this descriptor = 7 bytes
  unsigned char bDescriptorType; // Descriptor type = ENDPOINT (05h)
  struct {
    unsigned char index : 4; // The endpoint number
    unsigned char : 3;
    unsigned char direction : 1; // Direction. Ignored for Control, 0 = OUT endpoint, 1 = IN endpoint
  } bEndpointAddress;
  struct {
    TransferType transferType : 2;
    SynchronizationType synchronizationType : 2;
    UsageType usageType : 2;
    unsigned char : 3;
  } bmAttributes;
  unsigned short wMaxPacketSize; // Maximum packet size for this endpoint
  unsigned char bInterval;       // Polling interval in milliseconds for interrupt endpoints
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
  void stall();
};

class UsbInterface {
public:
  UsbDevice *device;
  UsbEndpoint *endpoints[MAX_ENDPOINTS + 1];

  virtual void init() {
    for (int c = 0; endpoints[c]; c++) {
      endpoints[c]->interface = this;
      endpoints[c]->device = device;
      endpoints[c]->init();
    }
  }
};

class UsbControlEndpoint : public UsbEndpoint {
public:
  unsigned char rxBuffer[64];
  unsigned char txBuffer[64];

  void setup(SetupData *setupData);

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
  UsbInterface *interfaces[MAX_INTERFACES + 1];

  UsbEndpoint *endpoints[MAX_ENDPOINTS];
  int endpointCount;

  UsbControlEndpoint controlEndpoint;

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
        UsbEndpoint *endpoint = interface->endpoints[e];
        endpoint->index = endpointCount++;
        endpoints[endpoint->index] = endpoint;
      }
    }
  }

  virtual void startTx(int epIndex, int length) = 0;
  virtual void stall(int epIndex) = 0;
  virtual void setAddress(int address) = 0;

  virtual void getDescriptor(DeviceDescriptor *deviceDesriptor) = 0;
};

void UsbEndpoint::startTx(int length) { device->startTx(index, length); }
void UsbEndpoint::stall() { device->stall(index); }

void UsbControlEndpoint::setup(SetupData *setupData) {

  if (setupData->bRequest == DEVICE_REQUEST_GET_DESCRIPTOR) {

    int descriptorType = setupData->wValue >> 8;
    int descriptorIndex = setupData->wValue & 0xFF;

    if (descriptorType == DESCRIPTOR_TYPE_DEVICE) {

      DeviceDescriptor *desriptor = (DeviceDescriptor *)txBufferPtr;
      memset(desriptor, 0, sizeof(DeviceDescriptor));

      desriptor->bLength = sizeof(DeviceDescriptor);
      desriptor->bDescriptorType = DESCRIPTOR_TYPE_DEVICE;
      desriptor->bcdUSB = 0x200;
      desriptor->bMaxPacketSize0 = sizeof(rxBuffer);
      desriptor->bNumConfigurations = 1;

      device->getDescriptor(desriptor);

      startTx(sizeof(DeviceDescriptor));

    } else if (descriptorType == DESCRIPTOR_TYPE_CONFIGURATION) {

      int totalLength = 0;

      ConfigurationDescriptor *configurationDescriptor = (ConfigurationDescriptor *)(txBuffer + totalLength);
      memset(configurationDescriptor, 0, sizeof(ConfigurationDescriptor));
      configurationDescriptor->bLength = sizeof(ConfigurationDescriptor);
      configurationDescriptor->bDescriptorType = DESCRIPTOR_TYPE_CONFIGURATION;
      configurationDescriptor->bConfigurationValue = 1;
      configurationDescriptor->bmAttributes.reservedSetTo1 = 1;
      configurationDescriptor->bMaxPower = 50;

      totalLength += sizeof(ConfigurationDescriptor);

      for (int i = 0; device->interfaces[i]; i++) {     

        if (setupData->wLength > sizeof(ConfigurationDescriptor)) {
          UsbInterface *interface = device->interfaces[i];

          InterfaceDescriptor *interfaceDescriptor = (InterfaceDescriptor *)(txBuffer + totalLength);
          memset(interfaceDescriptor, 0, sizeof(InterfaceDescriptor));
          interfaceDescriptor->bLength = sizeof(InterfaceDescriptor);
          interfaceDescriptor->bDescriptorType = DESCRIPTOR_TYPE_INTERFACE;
          interfaceDescriptor->bInterfaceNumber = i;

          totalLength += sizeof(InterfaceDescriptor);
        }

        configurationDescriptor->bNumInterfaces++;
      }

      configurationDescriptor->wTotalLength = totalLength;
      startTx(totalLength);

    } else {
      stall();
    }

  } else if (setupData->bRequest == DEVICE_REQUEST_SET_ADDRESS) {

    device->setAddress(setupData->wValue);
    startTx(0);

  } else if (setupData->bRequest == DEVICE_REQUEST_GET_CONFIGURATION) {

    txBuffer[0] = 1;
    startTx(1);

  } else if (setupData->bRequest == DEVICE_REQUEST_SET_CONFIGURATION) {

    startTx(0);

  } else {
    stall();
  }
}

} // namespace usbd
