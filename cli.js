const { program } = require('commander');
const I2C = require("./i2c.js");

async function action(asyncAction) {
    try {
        let connectionString = program.opts().connection || process.env.I2C;
        let i2c = await I2C(connectionString);

        try {
            await asyncAction(i2c);
        } finally {
            await i2c.close();
        }

    } catch (e) {
        console.error("Error:", e.message || e);
        process.exit(1);
    }
}

program.version(require("./package.json").version);

program.option("-c, --connection <value>", "connection string, defaults to I2C environment variable");

program
    .command("read <address> <length>")
    .description("Read data from I²C device")
    .action((address, length) => action(async i2c => {
        let data = await i2c.i2cRead(parseInt(address), parseInt(length));
        console.info(data.toString("hex").toUpperCase());
    }));

program
    .command("write <address> <data>")
    .description("Write data to I²C device")
    .action((address, data) => action(async i2c => {
        await i2c.i2cWrite(parseInt(address), Buffer.from(data, "hex"));
    }));

program.command("scan")
    .description("Scan I²C bus for devices")
    .action(() => action(async i2c => {
        for (let address = 0x08; address < 0x78; address++) {
            try {
                await i2c.i2cRead(address, 1);
                console.info("0x" + address.toString(16).padStart(2, "0").toUpperCase());
            } catch (e) {
            }
        }
        console.info("Done.")
    }));

function noPin(pin) {
    throw new Error(`Unknown pin ${pin}`);
}

program
    .command("get <pin>")
    .description("Get value of the named GPIO pin")
    .action((pin) => action(async i2c => {
        const getter = (i2c["get" + pin] || noPin(pin)).bind(i2c);
        console.info(await getter());
    }));


program
    .command("set <pin> <value>")
    .description("Set value of the named GPIO pin")
    .action((pin, value) => action(async i2c => {
        const setter = (i2c["set" + pin] || noPin(pin)).bind(i2c);
        await setter(value === "1" || value === "true");
    }));

program
    .command("wait <pin>")
    .description("Wait for interrupt on the named GPIO pin")
    .action((pin) => action(async i2c => {
        const onInterrupt = (i2c["on" + pin] || noPin(pin)).bind(i2c);
        let promise = new Promise(resolve => onInterrupt(resolve));
        await i2c.nop();
        console.info(`Waiting for ${pin}...`);
        await promise;
        console.info(`${pin}`);
    }));

program.parse();




