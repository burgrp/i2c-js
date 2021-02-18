const HID = require("node-hid");
const { checkRead, checkWrite } = require("./common.js");
const CriticalSection = require("promise-critical-section");

module.exports = async ({
    vid = 0x04D8,
    pid = 0x00DD,
    irq: gpIrq = 0,
    trg: gpTrg = 1,
    noAct: disableLedI2C = true
}) => {

    let deviceCriticalSection = new CriticalSection();

    gpIrq = gpIrq && parseInt(gpIrq);
    gpTrg = gpTrg && parseInt(gpTrg);

    let device = new HID.HID(parseInt(vid), parseInt(pid));

    let pendingRequest;

    device.on("data", function (data) {
        if (pendingRequest && data[0] === pendingRequest.code) {
            pendingRequest.cb(data);
        } else {
            console.info("out of band data", data);
        }
    });

    device.on("error", function (error) {
        console.info("error", error);
    });

    async function command(request) {
        deviceCriticalSection.enter();
        try {
            request = [...request, ...new Array(64 - request.length).fill(0)];
            let promise = new Promise((resolve, reject) => {
                let timeout = setTimeout(() => {
                    reject(`I2C command time out`);
                }, 2000);

                pendingRequest = {
                    code: request[0],
                    cb(response) {
                        clearTimeout(timeout);
                        resolve(response);
                    }
                }

            });
            device.write(request);
            return await promise;
        } finally {
            deviceCriticalSection.leave();
        }
    }

    let triggerState = 1;

    async function updateGpio() {

        let request = [
            0x60, 0, 0, 0, 0, 0, 0,
            1 << 7, // 7: alter GP designation
            4 << 1, // GP0, by default output/hi
            4 << 1, // GP1, by default output/hi
            4 << 1, // GP2, by default output/hi
            disableLedI2C ? 4 << 1 : 1  // GP3, I2C LED, if enabled, output/hi otherwise
        ];

        request[gpIrq + 8] = 1 << 3; // input
        request[gpTrg + 8] = triggerState ? 1 << 4 : 0; // output


        await command(request);

    }

    await updateGpio();


    return {
        async read(address, length) {
            let buffer = Buffer.alloc(length);
            let read = await bus.i2cRead(parseInt(address), length, buffer);
            checkRead(read, length);
            return Uint8Array.from(buffer);
        },

        async write(address, data) {
            let buffer = Buffer.from(data);
            let written = await bus.i2cWrite(parseInt(address), data.length, buffer);
            checkWrite(written, data.length);
        },

        async trigger(state) {
            triggerState = state;
            await updateGpio();
        },

        async close() {
            await bus.close();
        }
    }
}