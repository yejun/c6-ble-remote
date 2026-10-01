# C6 BLE Remote

Control an ESP32-C6's onboard RGB LED and read its temperature and BOOT button from the browser over Bluetooth LE (Web Bluetooth).

**Open the page:** https://yejun.github.io/c6-ble-remote/

## Firmware

Flash `firmware/ble_demo` with the Arduino ESP32 core (3.x):

```
arduino-cli compile --fqbn esp32:esp32:esp32c6:CDCOnBoot=cdc --upload -p /dev/ttyACM0 firmware/ble_demo
```

The board advertises as `C6-BLE-Demo`.

## Browser support

Chrome or Edge on desktop or Android. Safari/iOS don't support Web Bluetooth.
On Linux, enable `chrome://flags/#enable-web-bluetooth` and restart Chrome.
