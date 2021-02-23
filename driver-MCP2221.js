const HID = require("node-hid");
const { checkRead, checkWrite } = require("./common.js");
const CriticalSection = require("promise-critical-section");
const logError = require("debug")("i2c:error");

module.exports = async ({
    vid = 0x04D8,
    pid = 0x00DD,
    irq: gpIrq = 0,
    trg: gpTrg = 1,
    noAct: disableLedI2C = false
}) => {

    let deviceCriticalSection = new CriticalSection();
    let irqState;
    let irqListener;

    gpIrq = gpIrq && parseInt(gpIrq);
    gpTrg = gpTrg && parseInt(gpTrg);

    let device = new HID.HID(parseInt(vid), parseInt(pid));

    let pendingRequest;

    // device.on("data", function (data) {
    //     if (pendingRequest && data[0] === pendingRequest.code) {
    //         pendingRequest.cb(data);
    //     } else {
    //         //logError(`Unexpected data when waiting for command 0x${(pendingRequest.code).toString(16).toUpperCase()}:`, data);
    //     }
    // });

    // device.on("error", function (error) {
    //     logError("Error", error);
    // });

    // async function command(request) {
    //     await deviceCriticalSection.enter();
    //     try {
    //         request = [...request, ...new Array(64 - request.length).fill(0)];
    //         let timeout;
    //         let promise = new Promise((resolve, reject) => {
    //             timeout = setTimeout(() => {
    //                 reject(`I2C command time out`);
    //             }, 2000);

    //             pendingRequest = {
    //                 code: request[0],
    //                 cb(response) {
    //                     clearTimeout(timeout);
    //                     resolve(response);
    //                 }
    //             }

    //         });

    //         try {
    //             device.write(request);
    //             return await promise;
    //         } catch (error) {
    //             // discard the timeout
    //             clearTimeout(timeout);
    //             promise.catch(() => { });
    //             throw error;
    //         }

    //     } finally {
    //         deviceCriticalSection.leave();
    //     }
    // }

    function command(request) {
        // await deviceCriticalSection.enter();
        // try {

            return new Promise((resolve, reject) => {

                request = [...request, ...new Array(64 - request.length).fill(0)];   
                device.write(request);

                device.read((error, data) => {
                    if (error) {
                        reject(`I2C adapter error: ${error}`);
                    } else {
                        resolve(data);
                    }
                })
    
               
            });

        // } finally {
        //     deviceCriticalSection.leave();
        // }
    }    

    let setGpioRequest = [
        0x60, 0, 0, 0, 0, 0,
        1 << 7 | 1 << 4 | 1 << 2 | 1 << 1,  // IRQ on falling edge
        1 << 7, // 7: alter GP designation
        1 << 3, // GP0, by default input
        1 << 3, // GP1, by default input
        1 << 3, // GP2, by default input
        disableLedI2C ? 1 << 3 : 1  // GP3, I2C LED, if enabled, output/hi otherwise
    ];

    setGpioRequest[gpIrq + 8] = 1 << 3; // input
    setGpioRequest[gpTrg + 8] = 1 << 4; // output/hi

    //await command(setGpioRequest);

    async function updateIrqState() {
        // try {
        //     let response = await command([0x51]);
        //     let state = response[2 + gpIrq * 2] === 1 ? false : true;
        //     let oldState = irqState;
        //     irqState = state;
        //     if (oldState !== irqState && irqState && irqListener) {
        //         await irqListener();
        //     }
        //     setTimeout(updateIrqState, 100);
        // } catch (e) {
        //     if (!device._closed) {
        //         logError("Error in IRQ handler:", e);
        //         setTimeout(updateIrqState, 1000);
        //     }
        // }
    }

    await updateIrqState();

    return {

        onIrq(listener) {
            irqListener = listener;
        },

        async read(address, length) {

            let r;

            //while (true) {

                r = await command([0x10, 0, 0x10]);
                console.info(r);
                await new Promise(r => setTimeout(r, 100));

                r = await command([0x10, 0, 0, 0x20, (12000000 / 100000) - 3]);
                console.info(r);
                await new Promise(r => setTimeout(r, 100));

//            }




            r = await command([0x91, length, 0, address << 1]);
            console.info(r);
            // if (response[1] !== 0) {
            //     //throw new Error(`I2C read failed with code ${response[1]}`);
            // }

            response = await command([0x40]);
            return response;
        },

        async write(address, data) {
            let buffer = Buffer.from(data);
            let written = await bus.i2cWrite(parseInt(address), data.length, buffer);
            checkWrite(written, data.length);
        },

        async trigger(state) {

            let request = [
                0x50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            ];

            request[2 + 4 * gpTrg] = 1;
            request[3 + 4 * gpTrg] = state ? 0 : 1;

            await command(request);
        },

        async close() {
            device.close();
        },

        async getIrq() {
            return irqState;
        }
    }
}