namespace usbd {

/**
 * @brief USB device base class
 *
 * This class is not subclassed directly by the application.
 * There must always be chip specific implementation as midclass.
 * This means there is a hierarchy like:
 * UsbDevice <- ChipAbcUsbDevice <- ApplicationXyzUsbDevice.
 *
 * Typical application subclass of a chip midclass (ATSAMD in this case) looks like:
 * ```cpp
 * class TestDevice : public atsamd::usbd::AtSamdUsbDevice {
 * public:
 *  TestInterface testInterface;
 *
 *  UsbControlEndpoint controlEndpoint;
 *
 *  UsbInterface *getInterface(int index) { return index == 0 ? &testInterface : NULL; };
 *
 *  UsbEndpoint *getControlEndpoint() { return &controlEndpoint; };
 *
 *  void checkDescriptor(DeviceDescriptor *deviceDesriptor) {
 *    deviceDesriptor->idVendor = 0xFEE0;
 *    deviceDesriptor->idProduct = 0x0001;
 *  };
 *
 * };
 * ```
 * 
 * Chip midclass must overload following methods:
 * - void init(); - must call UsbDevice::init()
 * - void setAddress(int address);
 * - void startTx(int epIndex, int length);
 * - void stall(int epIndex);
*/
class UsbDevice {
public:
  /**
   * Array of endpoints as collected by the init() method.
   */
  UsbEndpoint *endpoints[MAX_ENDPOINTS];

  /**
   * Number of endpoints as collected by the init() method.
   */
  int endpointCount;

  /**
   * Device initialization.
   *
   * Called from chip midclass init() method.
   *
   * UsbEndpoint::init() is responsible for collecting of endpoints from interfaces.
   */
  virtual void init() {

    UsbEndpoint *controlEndpoint = getControlEndpoint();

    controlEndpoint->device = this;
    controlEndpoint->init();

    for (int i = 0; UsbInterface *interface = getInterface(i); i++) {
      interface->device = this;
      interface->init();
    }

    endpointCount = 0;
    controlEndpoint->index = endpointCount++;
    endpoints[controlEndpoint->index] = controlEndpoint;

    for (int i = 0; UsbInterface *interface = getInterface(i); i++) {
      for (int e = 0; UsbEndpoint *endpoint = interface->getEndpoint(e); e++) {
        endpoint->index = endpointCount++;
        endpoints[endpoint->index] = endpoint;
      }
    }
  }

  /**
   * Gets control endpoint.
   *
   * Called internally from the library.
   *
   * Must be overloaded by the application subclass.
   *
   * @return UsbEndpoint*
   */
  virtual UsbEndpoint *getControlEndpoint() = 0;

  /**
   * Gets interface by index.
   *
   * Called internally from the library.
   *
   * Must be overloaded by the application subclass.
   *
   * @param index zero based in scope of device
   * @return UsbInterface*
   */
  virtual UsbInterface *getInterface(int index) = 0;

  /**
   * Handler of device control request.
   *
   * Called internally from the library.
   *
   * May be overloaded by application subclass to implement vendor request.
   *
   * @param setupData
   */
  virtual void setup(SetupData *setupData){};

  /**
   * Called when generating device descriptor.
   *
   * Called internally from the library.
   *
   * May be overloaded by application subclass to modify the default descriptor.
   *
   * @param deviceDesriptor
   */
  virtual void checkDescriptor(DeviceDescriptor *deviceDesriptor){};

  /**
   * Get string identifier of the manufacturer.
   *
   * Called internally from the library.
   *
   * May be overloaded by application subclass.
   *
   * @return const char*
   */
  virtual const char *getManufacturer() { return NULL; };

  /**
   * Get string identifier of the product.
   *
   * Called internally from the library.
   *
   * May be overloaded by application subclass.
   *
   * @return const char*
   */
  virtual const char *getProduct() { return NULL; };

  /**
   * Get string serial number of the device.
   *
   * Called internally from the library.
   *
   * May be overloaded by application subclass or chip midclass.
   *
   * @return const char*
   */
  virtual const char *getSerial() { return NULL; };

  /**
   * Sets device USB address.
   *
   * Called internally from the library.
   *
   * Must be overloaded by chip midclass.
   *
   * @param address
   */
  virtual void setAddress(int address) = 0;

  /**
   * Starts transmission of the data on the given endpoint.
   *
   * Called internally from the library.
   *
   * Must be overloaded by chip midclass.
   *
   * @param epIndex
   * @param length
   */
  virtual void startTx(int epIndex, int length) = 0;

  /**
   * Sends stall handshake on the given endpoint.
   *
   * Called internally from the library.
   *
   * Must be overloaded by chip midclass.
   *
   * @param epIndex
   */
  virtual void stall(int epIndex) = 0;
};

void UsbEndpoint::startTx(int length) { device->startTx(index, length); }
void UsbEndpoint::stall() { device->stall(index); }

} // namespace usbd
