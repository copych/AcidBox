# AcidBox
ESP32 headless acid combo of 2 x 303 + 1 x 808 like synths.
MIDI driven. I2S output. No indication. Uses both cores of ESP32.

# To build the thing
You will need an ESP32 with PSRAM (ESP32 WROVER module). Preferrable an external DAC, like PCM5102. In ArduinoIDE select: 
* board: ESP32 Dev Module
* partition scheme: NoOTA (1MB Prog/ 3MB SPDIFF)
* PSRAM: enabled

Also you will need to upload samples to the ESP32 flash. To do so go there: https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool
