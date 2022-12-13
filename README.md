# AcidBox
ESP32 headless acid combo of 2 x 303 + 1 x 808 like synths.
MIDI driven. I2S output. No indication. Uses both cores of ESP32. Cheap ~$10.
Consult with midi_config.h to find out and to set up MIDI continous control messages.

# JukeBox
If you compile with #define JUKEBOX option in config.h, this becomes a stand-alone ACID-JUKE-BOX. You just listen.
A modified version of http://tips.ibawizard.net/acid-banger/, initially taken from https://github.com/vitling/acid-banger included, there's no AI, but randomizing pattern algorhythms.

# To build the thing
You will need an ESP32 with PSRAM (ESP32 WROVER module). Preferrable an external DAC, like PCM5102. In ArduinoIDE select: 
* board: ESP32 Dev Module
* partition scheme: No OTA (1MB APP/ 3MB SPDIFF)
* PSRAM: enabled

Also you will need to upload drum samples to the ESP32 flash (LittleFS). To do so follow the instructions: https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool

<img src="https://github.com/copych/AcidBox/blob/main/hardware/2022-12-13%2015-44-53.JPG" width=500>

# Thanks go to
* Marcel Licence https://github.com/marcel-licence : synth and DSP related code (delay, bi-filter, bitcrusher initially were taken from here)
* Infrasonic Audio https://github.com/infrasonicaudio : synth and DSP related code (moogladder filter, wavefolder and i2s base code were initially taken from here)
* Erich Heinemann https://github.com/ErichHeinemann : forked Marcel Licence's sampler and drum samples were taken from here
