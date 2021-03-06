const I2C = require("./i2c.js");

async function start() {

    let i2c = await I2C("usb,serial=IORL7JPK2184EEEF");

    try {
        while (true) {
            // await i2c.trigger(true);
            // await i2c.trigger(false);
            console.info("R", await i2c.i2cRead(0x4F, 2));

            try {
            console.info("W");
            await i2c.i2cWrite(0x4F, Buffer.from("ABC"));
            } catch(e) {
                console.error("WE", e);
            }
            await new Promise(resolve => setTimeout(resolve, 500));
        }
    } finally {
        await i2c.close();
    }
}

start().catch(e => console.error(e));