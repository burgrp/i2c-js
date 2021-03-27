const I2C = require("../i2c.js");

/**
 * This example reads temperature from LM75A sensor.
 * Please check the address - 0x4F means all address pins are pulled up.
 * Furthermore we configure output pin 5 connected to LED 
 * and we turn on the LED if the temperature is above 25°C.
 */

async function start() {
    let i2c = await I2C("drake,gpioLED=5.out");
    try {

        let data = await i2c.i2cRead(0x4F, 2);
        let temperature = data.readInt16BE(0) / 256;

        console.info(`It is ${temperature}°C`);

        await i2c.setLED(temperature > 25);

    } finally {
        await i2c.close();
    }
}

start().catch(e => console.error(e));