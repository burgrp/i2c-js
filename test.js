const I2C = require("./i2c.js");

async function start() {

    console.info("Opening I2C interface...");
    let i2c = await I2C("usb,serial=IORL7JPK2184EEEF,gpioTrigger=5.out.openDrain.pullUp,gpioIrq=6.in.pullUp.irqFallingEdge");

    i2c.onIrq(() => {
        console.info(`IRQ!`);
    });


    while (true) {
        try {
            await i2c.nop();
            console.info("R", await i2c.i2cRead(0x4F, 2));
            console.info("W");
            await i2c.i2cWrite(0x4F, Buffer.from("ABC"));

            await i2c.setTrigger(0);
            await i2c.setTrigger(1);
            console.info("IRQ:", await i2c.getIrq());
        } catch (e) {
            console.error(e);
        }
        //await new Promise(resolve => setTimeout(resolve, 500));
    }

}

start().catch(e => console.error(e));