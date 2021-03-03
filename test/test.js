const usb = require("usb");

async function start() {

    let device = usb.findByIds(0xFEE0, 0x0001);

    await device.open();

    try {

        function ledMode(mode) {

            return new Promise((resolve, reject) => {
                device.controlTransfer(
                    usb.LIBUSB_ENDPOINT_OUT | usb.LIBUSB_REQUEST_TYPE_CLASS | usb.LIBUSB_RECIPIENT_INTERFACE,
                    0x10,
                    mode,
                    0,
                    Buffer.from([]), (error, data) => {
                        if (error) {
                            reject(error);
                        } else {
                            resolve(data);
                        }
                    });
    
            })
        }

        function wait(ms) {
            console.info(`Waiting ${ms} ms...`);
            return new Promise(resolve => setTimeout(resolve, ms));
        }

        function getCRC(data) {
            return new Promise((resolve, reject) => {
                let interface = device.interface(0);
                interface.claim();
                let outEndpoint = interface.endpoint(0x01);
                let inEndpoint = interface.endpoint(0x81);
                
                outEndpoint.transfer(Buffer.from(data), error => {
                    if (error) {
                        reject(error);
                    } else {
                        inEndpoint.transfer(4, (error, data) => {
                            if (error) {
                                reject(error);
                            } else {
                                resolve(data.readUInt32LE(0));
                            }
                        });
                    }
                });
            });
        }

        console.info("LED OFF");
        await ledMode(0);
        await wait(2000);

        console.info("LED ON");
        await ledMode(1);
        await wait(2000);

        console.info("LED BLINK");
        await ledMode(2);

        let data = new Array(1024).fill(0).map((n,i) => i & 0xFF); //Math.floor(Math.random() * 256));
        let myCRC = data.reduce((acc, n) => (acc + n) & 0xFFFFFFFF, 0);
        let deviceCRC = await getCRC(data);

        console.info(`My CRC: ${myCRC}, device CRC ${deviceCRC}`);

    } finally {
        await device.close();
    }
}


start().catch(e => {
    console.error(e);
    process.exit(1);
});