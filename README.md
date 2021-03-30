# I2C-JS

JavaScript binding for I²C with support for multiple drivers.

To support auxiliary signals (such as interrupts), the library allows GPIO configuration, reading and writing.

## Example

```js
const I2C = require("@burgrp/i2c");

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
```

## Configuration

The library uses a connection string to select and configure the hardware I²C interface and GPIO.

The connection string selects the proper driver and further configures the driver if necessary.

Format of the connection string is:
```
driver,property1=value,property2=value,...`
```

### Implemented drivers

#### driver `sys`

<!-- 
There is a driver to access I²C and GPIO provided by operating system / kernel. This is the driver used on Raspberry PI like computers. The driver just delegates calls to JavaScript libraries ....
-->

__TBD__

#### driver `drake`

There is a driver for [Drake USB-I2C bridge](https://github.com/burgrp/hw-USB-I2C), which is handy while developing or debugging on host computer. If not specified otherwise, the driver looks up for device 1209:7070. 

##### Driver options
- vid - VID of the device, optional, defaults to "1209"
- pid - PID of the device, optional, defaults to "7070"
- serial - Serial number of the device, optional

##### Example
```
drake,serial=C639ADEE2184FFFF,gpioIrq=5.in.pullUp.irqFallingEdge
```

This selects the Drake USB-I2C bridge by given serial number (thus allows to distinguish between multiple connected bridges) and configures pin 5 as pull-up input rising interrupts on falling edge.

### GPIO configuration

There is a generic, driver unified, way to configure GPIO pins. 

On library API, pins are referred by pin names rather than physical pin numbers. This allows higher level of abstraction. Properties of the connection string starting with `gpio` prefix are treated as GPIO configuration. The connection string can contain multiple GPIO configurations.

Part of the property key after `gpio` prefix is the GPIO name. 

Value of the `gpio?` property is a string with options. First two mandatory fields are pin number and direction. There are other options, which depends on pin direction.

This is an example of USB interface configured with two pins, first one called "Trigger" mapped to pin 6, which is open drain output with internal pull-up enabled and the second pin is called "Irq" mapped to pin 5 which is input with internal pull-up and firing interrupt on falling edge.

```
usb,gpioTrigger=6.out.openDrain.pullUp,gpioIrq=5.in.pullUp.irqFallingEdge
```

#### options of output pins
- openDrain - configure pin for open drain mode, if not set pin is configured as push-pull
- pullUp - enable internal pull-up resistor; this option is valid only in open-drain mode

#### options of input pins
- pullDown - enable internal pull-down resistor
- pullUp - enable internal pull-up resistor
- irqRisingEdge - fire interrupt on rising edge
- irqFallingEdge - fire interrupt on falling edge

Options pullDown and pullUp can not be set simultaneously.

Options irqRisingEdge and irqFallingEdge can be set simultaneously, in which case the interrupt is fired on any edge.

## API

Library main module returns factory function which takes connection string as argument:
```js
const I2C = require("@burgrp/i2c");
let i2c = await I2C("drake,gpioLED=5.out");
// now we have the i2c bus
```

### I2C operations

I2C read and write operations are straightforward:

```js
let buffer = await i2c.i2cRead(address, length);
```

```js
await i2c.i2cWrite(address, buffer);
```

### GPIO operations

Although there are two generic R/W functions available (`async gpioRead(pinNumber)` and `async gpio(pinNumber, value)`), preferred way is to use dynamically generated getters and setters named after pins:

With connection string as follows:
```js
const I2C = require("@burgrp/i2c");
let i2c = await I2C("drake,gpioLED=5.out,gpioSignal=4.in.irqFallingEdge");
```
we have two pins: output called `LED` and input called `Signal`.

Then there is dynamically generated setter for `LED` pin:
```js
await i2c.setLED(true); // turn the LED on
await i2c.setLED(false); // turn the LED on
```

And there is dynamically generated getter and interrupt handler registration for `Signal` pin:

```js
let signalValue = await i2c.getSignal();
```

```js
i2c.onSignal(async () => {
    console.info("Signal received!");
});
```

### Closing the device
Always call `close` asynchronously. It may take some time to close the device due to pending IRQ event listener.
```js
await i2c.close();
```

## CLI

I2C-JS comes with simple command line tool `i2c`:

```
Usage: cli [options] [command]

Options:
  -V, --version             output the version number
  -c, --connection <value>  connection string, defaults to I2C environment variable
  -h, --help                display help for command

Commands:
  read <address> <length>   Read data from I²C device
  write <address> <data>    Write data to I²C device
  scan                      Scan I²C bus for devices
  get <pin>                 Get value of the named GPIO pin
  set <pin> <value>         Set value of the named GPIO pin
  wait <pin>                Wait for interrupt on the named GPIO pin
  help [command]            display help for command
```

CLI can be installed globally as follows:
```shell
sudo npm install --unsafe-perm --global @burgrp/i2c
```
