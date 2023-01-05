# SFM-V1.7 Fingerprint Driver for ESP32-Arduino

* Provides support for "SFM-V1.7" Fingerprint module.

* See examples and source code for more detailed information.

## Notice

* If your MCU's GPIO port can't provide heavy current, I suggest connecting the SFM_VCC (Fingerprint VCC, green wire) directly to the 3.3V VIN without controlling the touch recognition part's power state, which does make a little bit more power consumption.

* The module works at 3.3V electricity level, which means connecting it directly to the 5V GPIO port may result in malfunction and/or circuit damage.