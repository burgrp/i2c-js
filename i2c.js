module.exports = async portStr => {

    if (!portStr) {
        throw new Error("I2C port needs to be specified");
    }

    let params = portStr.split(",").map(p => p.trim());
    let driver = params.shift();

    params = params.map(p => p
        .split("=")
        .map(s => s.trim())
    ).reduce((acc, [k, v]) => ({...acc, [k]: v === undefined? true: v}), {});

    return await require(`./driver-${driver}.js`)(params);
}