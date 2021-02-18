const Bus = require("i2c-bus-promise");
const { checkRead, checkWrite } = require("./common.js");

module.exports = async ({bus: busNo = 0}) => {

    const bus = await Bus.openPromisified(parseInt(busNo));

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

        async close() {
            await bus.close();
        }
    }
}