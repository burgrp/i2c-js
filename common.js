module.exports = {

    checkRead(actual, wanted) {
        if (actual !== wanted) {
            throw new Error(`Could read only ${actual} bytes from ${wanted}`);
        }
    },

    checkWrite(actual, wanted) {
        if (actual !== wanted) {
            throw new Error(`Could write only ${actual} bytes from ${wanted}`);
        }
    }

}