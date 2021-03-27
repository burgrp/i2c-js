const I2C = require("../i2c.js");

async function start() {

    let i2c = await I2C("drake,gpioTrigger=4.out.openDrain.pullUp,gpioIrq=5.in.pullUp.irqFallingEdge");

    i2c.onIrq(() => {
        console.info(`IRQ!`);
    });

    while (true) {
        
        try {
            
            console.info("R", await i2c.i2cRead(0x4F, 2));
            console.info("W");
            await i2c.i2cWrite(0x4F, Buffer.from("ABC"));

            await i2c.setTrigger(!await i2c.getTrigger());
            console.info("IRQ:", await i2c.getIrq());
        } catch (e) {
            console.error(e);
        }
        
        await new Promise(resolve => setTimeout(resolve, 100));
    }

}

start().catch(e => console.error(e));