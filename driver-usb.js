const usb = require("usb");
const { checkRead, checkWrite } = require("./common.js");

const REQUEST_GPIO_CONFIGURE = 1;
const REQUEST_GPIO_READ = 2;
const REQUEST_GPIO_WRITE = 3;

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

    let needsReopen;
    let irqHandler;

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

    async function readIrq() {
        while (true) {
            try {
                let data = await irqIn.transfer(4);
                if (irqHandler) {
                    await irqHandler(data.readUInt32LE(0));
                }
            } catch (e) {
                if (e.message !== "LIBUSB_TRANSFER_TIMED_OUT") {
                    needsReopen = e;
                    break;
                }
            }
        }
    }

    readIrq();

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

    function interfaceCtrlRequestOut(requestId, value, data = Buffer.from([])) {
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

        async configureGpio(gpioConfig) {

            for (let gpio of gpioConfig) {
                await interfaceCtrlRequestOut(REQUEST_GPIO_CONFIGURE,
                    gpio.index |
                    (
                        gpio.direction === "out" ?
                            1 << 8 |
                            (gpio.setting.openDrain ? 1 : 0) << 9 |
                            (gpio.setting.pullUp ? 1 : 0) << 10
                            :
                            0 << 8 |
                            (gpio.setting.pullDown ? 1 : 0) << 9 |
                            (gpio.setting.pullUp ? 1 : 0) << 10 |
                            (gpio.setting.irqRisingEdge ? 1 : 0) << 11 |
                            (gpio.setting.irqFallingEdge ? 1 : 0) << 12
                    )
                );

            }
        },

        async gpioRead(pin) {
            return (await interfaceCtrlRequestIn(REQUEST_GPIO_READ, pin, 8)).readUInt8(0) === 1;
        },

        async gpioWrite(pin, state) {
            await interfaceCtrlRequestOut(REQUEST_GPIO_WRITE, pin | (state ? 0x100 : 0x00));
        },

        async onIrq(handler) {
            irqHandler = handler;
        },

        async close() {
            device.close();
        },

        async driverNeedsReopen() {
            return needsReopen;
        },

        async errorNeedsReopen(error) {
            return error.message === "LIBUSB_ERROR_NO_DEVICE";
        }
    }
}