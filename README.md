# I2C-JS

## Example

Read temperature from LM75A sensor:

```js
TBD
```

## API

### async i2cRead(address, length)


### async i2cWrite(address, data)

### async gpioConfigureInput(pin, options)
Configures GPIO pin as input.
#### parameters:
##### pin
number of pin to configure 
##### options
- **pullUp**: `true` to enable pull-up 
- **pullDown**: `true` to enable pull-down 
- **irqRisingEdge**: `true` to enable IRQ on rising edge
- **irqFallingEdge**: `true` to enable IRQ on falling edge
- **irqHandler**: IRQ handler(state, pin) to call

### async gpioReadInput(pin)
Get logic level on pin.
#### parameters:
##### pin
number of pin to read 

### async gpioConfigureOutput(pin)
Configures GPIO pin as output.
#### parameters:
##### pin
number of pin to configure 

### async gpioWriteOutput(pin, state)
Sets tri-state level on pin. 
#### parameters:
##### pin
number of pin to set
##### state 
`true` for hi, `false` for low, `undefined` for hi-z

### async close()
closes the device

## Installation

```sh
npm -g i i2c-js
```

## CLI