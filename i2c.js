const CriticalSection = require("promise-critical-section");
const Debug = require("debug")("i2c");

let log = ["debug", "info", "warn", "error"]
    .reduce((acc, level) => ({ ...acc, [level]: Debug.extend(level) }));

module.exports = async portStr => {

    if (!portStr) {
        throw new Error("I2C port needs to be specified");
    }

    let params = portStr.split(",").map(p => p.trim());
    let name = params.shift();

    params = params.map(p => p
        .split("=")
        .map(s => s.trim())
    ).reduce((acc, [k, v]) => ({ ...acc, [k]: v === undefined ? true : v }), {});

    let criticalSection = new CriticalSection();

    let driver;

    return ["i2cRead", "i2cWrite", "close"].reduce((acc, method) => ({
        ...acc, [method]: async (...args) => {

            await criticalSection.enter();

            try {

                if (!driver) {
                    log.info("Opening driver")
                    driver = await require(`./driver-${name}.js`)(params);
                }

                try {
                    return await driver[method](...args);
                } catch (e) {
                    if (await driver.needsReopen(e)) {
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
    }), {});
}