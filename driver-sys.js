const { checkRead, checkWrite } = require("./common.js");

module.exports = async ({ bus: busNumber }) => {

    const bus = await require("i2c-bus").openPromisified(parseInt(busNumber) || 0);

    let irqHandler;

    return {

        async i2cRead(address, length) {
            let buffer = Buffer.alloc(length);
            checkRead((await bus.i2cRead(parseInt(address), length, buffer)).bytesRead, length);
            return buffer;
        },

        async i2cWrite(address, data) {
            checkWrite((await bus.i2cWrite(parseInt(address), data.length, data)).bytesWritten, length);
        },

        async configureGpio(gpioConfig) {

            if (gpioConfig.length) {
                throw new Error("GPIO not supported on sys driver yet");
            }

            for (let gpio of gpioConfig) {
                // await interfaceCtrlRequestOut(REQUEST_GPIO_CONFIGURE,
                //     gpio.index |
                //     (
                //         gpio.direction === "out" ?
                //             1 << 8 |
                //             (gpio.setting.openDrain ? 1 : 0) << 9 |
                //             (gpio.setting.pullUp ? 1 : 0) << 10
                //             :
                //             0 << 8 |
                //             (gpio.setting.pullDown ? 1 : 0) << 9 |
                //             (gpio.setting.pullUp ? 1 : 0) << 10 |
                //             (gpio.setting.irqRisingEdge ? 1 : 0) << 11 |
                //             (gpio.setting.irqFallingEdge ? 1 : 0) << 12
                //     )
                // );                
            }
        },

        async gpioRead(pin) {
            // return (await interfaceCtrlRequestIn(REQUEST_GPIO_READ, pin, 8)).readUInt8(0) === 1;
        },

        async gpioWrite(pin, state) {
            // await interfaceCtrlRequestOut(REQUEST_GPIO_WRITE, pin | (state ? 0x100 : 0x00));
        },

        async onIrq(handler) {
            irqHandler = handler;
        },

        async close() {
            await bus.close();
        },

        async driverNeedsReopen() {
            return false;
        },

        async errorNeedsReopen(error) {
            return false;
        }
    }
}


//     const pin = 198;

//     const dir = `/sys/class/gpio/gpio${pin}`;

//     try {
//         await fs.promises.writeFile("/sys/class/gpio/unexport", pin.toString());
//     } catch {
//         // fall through
//     }

//     await fs.promises.writeFile("/sys/class/gpio/export", pin.toString());

//     await fs.promises.writeFile(`${dir}/direction`, "in");
//     await fs.promises.writeFile(`${dir}/edge`, "falling");

//     let interruptCallback;

//     async function readInterrupt() {
//         try {
//             let stateStr = await fs.promises.readFile(`${dir}/value`);
//             let pending = stateStr.toString().trim() === "0";
//             //console.info("INT: ", stateStr, pending,interruptCallback);
//             if (pending && interruptCallback) {
//                 let cb = interruptCallback;
//                 interruptCallback = undefined;
//                 try {
//                     cb();
//                 } catch (e) {
//                     error("Error in I2C alert callback:", e);
//                 }
//             }
//         } catch (e) {
//             error("Error in I2C alert check:", e);
//         }
//         setTimeout(readInterrupt, 50);
//     }
