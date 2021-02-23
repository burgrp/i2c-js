const I2C = require("./i2c.js");

async function start() {

    let i2c = await I2C("MCP2221");

    i2c.onIrq(() => {
        console.info("IRQ");
    });

    try {
        while (true) {
            // await i2c.trigger(true);
            // await i2c.trigger(false);
            console.info(await i2c.read(0x4F, 2));

            await new Promise(resolve => setTimeout(resolve, 500));
        }
    } finally {
        await i2c.close();
    }
}

start().catch(e => console.error(e));