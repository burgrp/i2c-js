namespace usbd {

/**
 * @brief USB interface base class
 *
 * Interface usually has one or more endpoints.
 *
 * Typical subclass of UsbInterface class looks like:
 * ```cpp
 * class TestInterface : public usbd::UsbInterface {
 * public:
 *   TestEndpoint testEndpoint;
 *
 *   virtual UsbEndpoint *getEndpoint(int index) { return index == 0 ? &testEndpoint : NULL; }
 *
 * };
 * ```
 */
class UsbInterface {
public:
  /**
   * Owner device.
   * This property is set internally by the library.
   */
  UsbDevice *device;

  /**
   * Interface initialization.
   *
   * Called internally from the library.
   *
   * UsbEndpoint::init() is responsible for calling init() method of endpoints.
   */
  virtual void init() {
    for (int e = 0; UsbEndpoint *endpoint = getEndpoint(e); e++) {
      endpoint->interface = this;
      endpoint->device = device;
      endpoint->init();
    }
  }

  /**
   * Gets endpoint by index.
   *
   * Must be overloaded by the subclass.
   *
   * Called internally from the library.
   *
   * @param index zero based in scope of interface
   * @return UsbEndpoint*
   */
  virtual UsbEndpoint *getEndpoint(int index) = 0;

  /**
   * Handler of interface control request.
   *
   * May be overloaded to implement vendor request.
   *
   * @param setupData
   */
  virtual void setup(SetupData *setupData){};

  /**
   * Called when generating interface descriptor.
   *
   * Called internally from the library.
   * 
   * May be overloaded to modify the default descriptor.
   *
   * @param interfaceDesriptor
   */
  virtual void checkDescriptor(InterfaceDescriptor *interfaceDesriptor){};

  /**
   * Get string identifier of the interface.
   *
   * May be overloaded by the subclass.
   *
   * @return const char*
   */
  virtual const char *getLabel() { return NULL; };
};

} // namespace usbd