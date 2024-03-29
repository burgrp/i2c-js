const CriticalSection = require("promise-critical-section");
const Debug = require("debug")("i2c");

let log = ["debug", "info", "warn", "error"]
    .reduce((acc, level) => ({ ...acc, [level]: Debug.extend(level) }), {});

module.exports = async portStr => {

    if (!portStr) {
        throw new Error("I2C port needs to be specified");
    }

    let params = portStr.split(",").map(p => p.trim());
    let name = params.shift();

    params = params.map(p => p
        .split("=")
        .map(s => s.trim())
    ).reduce((acc, [k, v]) => ({
        ...acc,
        [k]: v === undefined ? true : v
    }), {});

    let gpio = Object.entries(params)
        .filter(([k, v]) => k.startsWith("gpio") && k !== "gpio")
        .map(([k, v]) => ({
            name: k.substring(4),
            index: parseInt(v.split(".")[0]),
            direction: v.split(".")[1] || "in",
            setting: v.split(".").slice(2).reduce((acc, k) => ({ ...acc, [k]: true }), {})
        }));

    let criticalSection = new CriticalSection();

    let driver;
    let irqHandlers = {};

    async function checkDriver() {

        let driverNeedsReopen;
        if (driver) {
            driverNeedsReopen = await driver.driverNeedsReopen();
            if (driverNeedsReopen) {
                log.error("Driver needs reopen because:", driverNeedsReopen.message || driverNeedsReopen);
            }
        }

        if (!driver || driverNeedsReopen) {
            log.info("Opening driver")
            driver = await require(`./driver-${name}.js`)(params);
            log.debug("Configuring GPIO:", gpio);
            await driver.configureGpio(gpio);

            driver.onIrq(async pinFlags => {
                for (let pin = 0; pin < 32; pin++) {                    
                    let handler = irqHandlers[pin];
                    if (handler && (pinFlags >> pin) & 1) {
                        try {
                            await handler();
                        } catch (e) {
                            log.error("Error in IRQ handler:", e);
                        }
                    }

                }
            });
        }
    }

    let wrapper = ["i2cRead", "i2cWrite", "gpioRead", "gpioWrite", "close"].reduce((acc, method) => ({
        ...acc,
        [method]: async (...args) => {

            await criticalSection.enter();

            try {

                await checkDriver();

                try {
                    log.debug(method, ...args);
                    return await driver[method](...args);
                } catch (e) {
                    if (await driver.errorNeedsReopen(e)) {
                        try {
                            log.info(`Closing driver because of error: ${e.message | e}`);
                            await driver.close();
                        } catch (e) {
                            log.warn("Could not close ")
                        }
                        driver = null;
                    }
                    throw e;
                }
            } finally {
                criticalSection.leave();
            }

        }
    }), {
        async nop() {
            await checkDriver();
        }
    });

    return gpio.reduce((acc, g) => ({
        ...acc,

        ["set" + g.name]: async function (value) {
            log.debug("set", g.name, value);
            await this.gpioWrite(g.index, value);
        },

        ["get" + g.name]: async function (value) {
            log.debug("get", g.name);
            return await this.gpioRead(g.index);
        },

        ["on" + g.name]: function (handler) {
            irqHandlers[g.index] = handler;
        }

    }), wrapper);
}