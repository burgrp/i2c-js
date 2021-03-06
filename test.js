const I2C = require("./i2c.js");

async function start() {

    console.info("Opening I2C interface...");
    let i2c = await I2C("usb,serial=IORL7JPK2184EEEF");

    while (true) {
        // await i2c.trigger(true);
        // await i2c.trigger(false);           

        try {
            console.info("R", await i2c.i2cRead(0x4F, 2));
            console.info("W");
            await i2c.i2cWrite(0x4F, Buffer.from("ABC"));
            // await Promise.all([
            //     i2c.i2cRead(0x4F, 2),
            //     i2c.i2cWrite(0x4F, Buffer.from("ABC"))
            // ]);
            console.info("RW");
        } catch (e) {
            console.error(e);
        }
        await new Promise(resolve => setTimeout(resolve, 500));
    }

}

start().catch(e => console.error(e));