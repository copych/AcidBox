# AcidBox
ESP32 headless acid combo of 2 x 303 + 1 x 808 like synths.
MIDI driven. I2S output. No indication. Uses both cores of ESP32. Cheap ~$10.
Consult with midi_config.h to find out and to set up MIDI continous control messages.

# It can be a JukeBox
If you compile with #define JUKEBOX option in config.h, this becomes a stand-alone ACID-JUKE-BOX. You just listen.
A modified version of http://tips.ibawizard.net/acid-banger/, initially taken from https://github.com/vitling/acid-banger included, there's no AI, but randomizing pattern algorithms.

[demo.mp3](https://github.com/copych/AcidBox/blob/main/media/acidjukebox.mp3?raw=true)
[demo2.mp3](https://github.com/copych/AcidBox/blob/main/media/acidjukebox2.mp3?raw=true)

# To build the thing
You will need an ESP32 with PSRAM (ESP32 WROVER module). Preferrable an external DAC, like PCM5102. 

<b>ATTENTION! PSRAM handling requires up-to-date Arduino ESP32 core. Of what I tested, versions 2.0.4, 2.0.5, 2.0.6 are working, 2.0.0 is not</b>

In ArduinoIDE (I've used v.1.8.20) select: 
* board: ESP32 Dev Module
* partition scheme: No OTA (1MB APP/ 3MB SPDIFF)
* PSRAM: enabled

Also you will need to upload drum samples to the ESP32 flash (LittleFS). To do so follow the instructions: https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool

# What if you only have WROOM module (no PSRAM)
You can still compile and run the app, with NO_PSRAM option in the config.h. Note, that in this case you will get NO DELAY, NO REVERB and just ONE DRUM KIT. 

In ArduinoIDE (I used v.1.8.20) select: 
* board: ESP32 Dev Module
* partition scheme: No OTA (1MB APP/ 3MB SPDIFF)
* PSRAM: disabled

Also you will need to upload drum samples to the ESP32 flash (LittleFS). To do so follow the instructions: https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool

# What if you don't have an external DAC module (PCM5102)
You can still compile and run the app, with USE_INTERNAL_DAC option in the config.h. BUT sound output is just terrible with internal DAC. Stereo 8-bit audio you can get form GPIO25 and GPIO26. Probably you can improve it a bit playing with the multipliers in i2s_output() method in general.ino file.

<b>ATTENTION! Built-in DAC works with Arduino ESP32 core 2.0.0, but not with newer versions (cause AFAIK they're still fixing that killing 2.0.1 update), so it may be a problem to get a working PSRAM + Internal DAC app</b>

In ArduinoIDE (I used v.1.8.20) select:
* board: ESP32 Dev Module
* partition scheme: No OTA (1MB APP/ 3MB SPDIFF)
* PSRAM: disabled

Also you will need to upload drum samples to the ESP32 flash (LittleFS). To do so follow the instructions: https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool



# Functional diagram 
<img src="https://github.com/copych/AcidBox/blob/main/media/2022-12-14_00-03-18.png" width=100%>
("Acid Banger" JukeBox actually calls midi functions as an external app)

# Thanks go to
* Marcel Licence https://github.com/marcel-licence : synth and DSP related code (delay, bi-filter, bitcrusher initially were taken from here)
* Infrasonic Audio https://github.com/infrasonicaudio : synth and DSP related code (moogladder filter, wavefolder and i2s base code were initially taken from here)
* Erich Heinemann https://github.com/ErichHeinemann : forked Marcel Licence's sampler and drum samples were taken from here

<img src="https://github.com/copych/AcidBox/blob/main/media/2022-12-13%2015-44-53.JPG" width=100%>
