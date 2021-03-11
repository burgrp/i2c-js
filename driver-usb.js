const usb = require("usb");
const { checkRead, checkWrite } = require("./common.js");

const REQUEST_GPIO_CONFIGURE_INPUT = 2;
const REQUEST_GPIO_READ_INPUT = 3;
const REQUEST_GPIO_CONFIGURE_OUTPUT = 4;
const REQUEST_GPIO_WRITE_OUTPUT = 5;

module.exports = async ({ vid = "1209", pid = "7070", serial }) => {

    let device;

    if (serial) {
        let devices = usb.getDeviceList();
        for (let d of devices) {
            d.open();
            try {
                try {
                    let deviceSerial = await new Promise((resolve, reject) => {
                        d.getStringDescriptor(d.deviceDescriptor.iSerialNumber, (error, data) => {
                            if (error) {
                                reject(error);
                            } else {
                                resolve(data);
                            }
                        });

                    });
                    if (deviceSerial === serial) {
                        device = d;
                    }
                } catch (e) {
                    // fall through
                }
            } finally {
                if (d !== device) {
                    d.close();
                }
            }
        }
    } else {
        device = usb.findByIds(parseInt(`0x${vid}`), parseInt(`0x${pid}`));
    }

    if (!device) {
        throw new Error("LIBUSB_ERROR_NO_DEVICE");
    }

    let interface = device.interfaces[0];
    interface.claim();

    interface.endpoints.forEach(ep => ep.timeout = 1000);
    let [i2cOut, i2cIn, irqIn] = interface.endpoints.map(ep => ({
        ...ep,
        transfer(lengthOrData) {
            return new Promise((resolve, reject) => {
                ep.transfer(lengthOrData, (error, data) => {
                    if (error) {
                        reject(error);
                    } else {
                        resolve(data);
                    }
                });
            });
        }
    }));

    function interfaceCtrlRequestIn(requestId, value, length) {
        return new Promise((resolve, reject) => {
            device.controlTransfer(
                0xC1, // interface vendor IN
                requestId,
                value,
                interface.interfaceNumber,
                length,
                (error, data) => {
                    if (error) {
                        reject(error);
                    } else {
                        resolve(data);
                    }
                }
            );
        });
    }

    function interfaceCtrlRequestOut(requestId, value, data) {
        return new Promise((resolve, reject) => {
            device.controlTransfer(
                0x41, // interface vendor OUT
                requestId,
                value,
                interface.interfaceNumber,
                data,
                (error) => {
                    if (error) {
                        reject(error);
                    } else {
                        resolve();
                    }
                }
            );
        });
    }

    return {

        async i2cRead(address, length) {
            let [, reply] = await Promise.all([
                i2cOut.transfer(Buffer.from([address << 1 | 1, length])),
                i2cIn.transfer(length)
            ]);
            checkRead(reply.length, length);
            return reply.slice(reply);
        },

        async i2cWrite(address, data) {
            let [, reply] = await Promise.all([
                i2cOut.transfer(Buffer.concat([Buffer.from([address << 1]), data])),
                i2cIn.transfer(1)
            ]);
            checkRead(reply.length, 1);
            checkWrite(data.length, reply[0]);
        },

        async gpioConfigureInput(pin, { pullUp, pullDown, irqRisingEdge, irqFallingEdge, irqHandler }) {
            await interfaceCtrlRequestOut(REQUEST_GPIO_CONFIGURE_INPUT,
                pin |
                (pullUp ? 1 : 0) << 8 |
                (pullDown ? 1 : 0) << 9 |
                (irqRisingEdge ? 1 : 0) << 10 |
                (irqFallingEdge ? 1 : 0) << 11
            );
        },

        async gpioReadInput(pin) {
            return (await interfaceCtrlRequestIn(REQUEST_GPIO_READ_INPUT, pin)).readUInt8(0) === 1;
        },

        async gpioConfigureOutput(pin) {
            await interfaceCtrlRequestOut(REQUEST_GPIO_CONFIGURE_OUTPUT, pin | state === true ? 1 : state === false ? 0 : 2);
        },

        async gpioWriteOutput(pin, state) {
            await interfaceCtrlRequestOut(REQUEST_GPIO_WRITE_OUTPUT, pin | state === true ? 1 : state === false ? 0 : 2);
        },

        async close() {
            device.close();
        }
    }
}