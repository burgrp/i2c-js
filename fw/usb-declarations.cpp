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

} // namespace usbd