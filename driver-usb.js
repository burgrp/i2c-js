const usb = require("usb");
const { checkRead, checkWrite } = require("./common.js");

const REQUEST_I2C_READ = 0;
const REQUEST_I2C_WRITE = 1;
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
        throw new Error("USB device not found");
    }

    let interface = device.interfaces[0];
    interface.claim();

    function requestIn(requestId, value, length) {
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

    function requestOut(requestId, value, data) {
        return new Promise((resolve, reject) => {
            device.controlTransfer(
                0x41, // interface vendor IN
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
            let data = await requestIn(REQUEST_I2C_READ, address, length);
            checkRead(data.length, length);
            return data;
        },

        async i2cWrite(address, data) {
            await requestOut(REQUEST_I2C_WRITE, address, data);
        },

        async gpioConfigureInput(pin, { pushPull: pullUpDown, irqRisingEdge, irqFallingEdge, irqHandler }) {
            await requestOut(REQUEST_GPIO_CONFIGURE_OUTPUT, 
                pin |
                (pullUpDown === true? 2: pullUpDown == false?1: 0) << 8 |
                (irqRisingEdge? 1: 0) << 10 |
                (irqFallingEdge? 1: 0) << 11 
            );
        },

        async gpioReadInput(pin) {

        },

        async gpioConfigureOutput(pin) {
            
        },

        async gpioWriteOutput(pin, state) {

        },

        async close() {
            device.close();
        }
    }
}