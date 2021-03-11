module.exports = {

    checkRead(actual, wanted) {
        if (actual !== wanted) {
            throw `Could read only ${actual} bytes from ${wanted}`;
        }
    },

    checkWrite(actual, wanted) {
        if (actual !== wanted) {
            throw `Could write only ${actual} bytes from ${wanted}`;
        }
    }

}