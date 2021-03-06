const Bus = require("i2c-bus-promise");
const { checkRead, checkWrite } = require("./common.js");

module.exports = async ({bus: busNo = 0}) => {

    const bus = await Bus.openPromisified(parseInt(busNo));

    return {

        async i2cRead(address, length) {
            let buffer = Buffer.alloc(length);
            let read = await bus.i2cRead(parseInt(address), length, buffer);
            checkRead(read, length);
            return buffer;
        },

        async i2cWrite(address, data) {
            let written = await bus.i2cWrite(parseInt(address), data.length, data);
            checkWrite(written, data.length);
        },

        async gpioConfigureInput(pin, { pullUpDown, irqRisingEdge, irqFallingEdge, irqHandler }) {
            throw new Error("Not implemented");            
        },

        async gpioReadInput(pin) {
            throw new Error("Not implemented");            
        },

        async gpioConfigureOutput(pin) {
            throw new Error("Not implemented");            
        },

        async gpioWriteOutput(pin, state) {
            throw new Error("Not implemented");            
        },

        async close() {
            await bus.close();
        }
    }
}