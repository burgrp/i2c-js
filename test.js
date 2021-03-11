const I2C = require("./i2c.js");

async function start() {

    while (true) {

        try {

            console.info("Opening I2C interface...");
            let i2c = await I2C("usb,serial=IORL7JPK2184EEEF");

            try {
                while (true) {
                    // await i2c.trigger(true);
                    // await i2c.trigger(false);           

                    try {
                        console.info("R", await i2c.i2cRead(0x4F, 2));
                        console.info("W");
                        await i2c.i2cWrite(0x4F, Buffer.from("ABC"));
                    } catch (e) {
                        if (e.message === "LIBUSB_ERROR_NO_DEVICE") {
                            break;
                        } else {
                            console.error("Error", e);
                        }
                    }
                    await new Promise(resolve => setTimeout(resolve, 500));
                }
            } finally {
                await i2c.close();
            }

        } catch (e) {
            if (e.message !== "LIBUSB_ERROR_NO_DEVICE") {
                throw e;
            }
        }

    }
}

start().catch(e => console.error(e));