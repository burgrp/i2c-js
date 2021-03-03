namespace usbd {

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
