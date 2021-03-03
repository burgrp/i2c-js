namespace usbd {

/**
 * @brief USB endpoint base class
 *
 * Endpoint has RX and TX buffer. The library (or the chip) handles multipacket
 * transfers.
 *
 * Buffers may be larger (but must be multiple) of related packet size.
 *
 * A subclass should set buffers in overloaded init() method.
 * For OUT endpoints, rxBufferPtr, rxBufferSize should be set.
 * For IN endpoints, txBufferPtr, txBufferSize  should be set.
 * For bidirectional endpoints, both RX and TX buffers should be set.
 *
 * Packet sizes may be set by rxPacketSize and txPacketSize properties, and may be
 * 8, 16, 32 or 64 bytes for control, interrupt and bulk transfers,
 * or 8, 16, 32, 64, 128, 256, 512 or 1023 for isochronous transfers.

 * If not set, packet sizes are derived from buffer sizes:
 * for buffers smaller than 64 bytes, packet size equals to buffer size.
 * For bigger buffers, packet size is 64 bytes.
 * 
 * Typical subclass of UsbEndpoint class looks like:
 * ```cpp
 * class TestEndpoint : public usbd::UsbEndpoint {
 * public:
 *   unsigned char rxBuffer[1024];
 *   unsigned char txBuffer[8];
 * 
 *   void init() {
 *     rxBufferPtr = rxBuffer;
 *     rxBufferSize = sizeof(rxBuffer);
 *     txBufferPtr = txBuffer;
 *     txBufferSize = sizeof(txBuffer);
 *     usbd::UsbEndpoint::init();
 *   }
 * 
 *   void rxComplete(int length) {
 * 
 *     // process rxBuffer data of the length
 * 
 *     // fill txBuffer and send some bytes
 *     startTx(4);
 *   }
 * };
 * ```
 */
class UsbEndpoint {
public:
  /**
   * Pointer do RX data buffer.
   * Should be set from overloaded init() method.
   */
  unsigned char *rxBufferPtr;

  /**
   * Size of RX data buffer.
   * Must be multiple of rxPacketSize.
   * Should be set from overloaded init() method.
   */
  unsigned int rxBufferSize;

  /**
   * Size of RX packet.
   * May be set from overloaded init() method.
   */
  unsigned int rxPacketSize;

  /**
   * Pointer do TX data buffer.
   * Should be set from overloaded init() method.
   */
  unsigned char *txBufferPtr;

  /**
   * Size of TX data buffer.
   * Must be multiple of txPacketSize.
   * Should be set from overloaded init() method.
   */
  unsigned int txBufferSize;

  /**
   * Size of TX data buffer.
   * Must be multiple of txPacketSize.
   * May be set from overloaded init() method.
   */
  unsigned int txPacketSize;

  /**
   * Transfer type of the endpoint.
   * May be set from overloaded init() method.
   * Defaults to INTERRUPT.
   */
  EndpointTransferType transferType = INTERRUPT;

  /**
   * Index of the endpoint in scope of the device.
   * This property is set internally by the library.
   */
  unsigned int index;

  /**
   * Owner interface.
   * This property is set internally by the library.
   */
  UsbInterface *interface;

  /**
   * Owner device.
   * This property is set internally by the library.
   */
  UsbDevice *device;

  /**
   * Endpoint initialization.
   *
   * Called internally from the library.
   *
   * UsbEndpoint::init() sets packet sizes based on buffer sizes, so
   * the subclass should call super method after setting of rxPacketSize and
   * txPacketSize properties.
   */
  virtual void init() {
    rxPacketSize = rxBufferSize > 64 ? 64 : rxBufferSize;
    txPacketSize = txBufferSize > 64 ? 64 : txBufferSize;
  }

  /**
   * Handler of endpoint control request.
   *
   * May be overloaded to implement vendor request.
   *
   * @param setupData
   */
  virtual void setup(SetupData *setupData) {}

  /**
   * Called when generating endpoint descriptor.
   *
   * May be overloaded to modify the default descriptor.
   *
   * @param deviceDesriptor
   */
  virtual void checkDescriptor(EndpointDescriptor *deviceDesriptor){};

  /**
   * Called when data received.
   *
   * May be overloaded to process received data.
   *
   * @param length
   */
  virtual void rxComplete(int length){};

  /**
   * Called when data transmitted.
   *
   * May be overloaded to recognize completed data transmission.
   */
  virtual void txComplete(){};

  /**
   * Starts transmission of the data.
   *
   * @param length
   */
  void startTx(int length);

/**
 * Sends stall handshake.
 */
  void stall();
};

} // namespace usbd