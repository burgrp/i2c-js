const I2C = require("./i2c.js");

async function start() {

    let i2c = await I2C("MCP2221");

    await i2c.trigger(0);
    await i2c.trigger(1);

    //console.info(await i2c.read(0x44, 1));

}

start().catch(e => console.error(e));