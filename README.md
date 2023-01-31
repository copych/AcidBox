# AcidBox
ESP32 headless acid combo of 2 x 303 + 1 x 808 like synths. Filter cutoff, reso, saturation, overdrive within each 303, hipass/lopass filter and bitcrusher in drums, send to reverb and delay and master compression.
All MIDI driven. 44100, 16bit stereo I2S output to the external DAC or 8bit to the built-in DAC. No indication. Uses both cores of ESP32. Cheap ~$10.
Consult with midi_config.h to find out and to set up MIDI continous control messages.

# It can be a JukeBox
If you compile with #define JUKEBOX option in config.h, this becomes a stand-alone ACID-JUKE-BOX. You just listen.
A modified version of http://tips.ibawizard.net/acid-banger/, initially taken from https://github.com/vitling/acid-banger included, there's no AI, but randomizing pattern algorithms.

# Sound samples
[demo.mp3](https://github.com/copych/AcidBox/blob/main/media/acidjukebox.mp3?raw=true) first time AcidBanger run

[demo2.mp3](https://github.com/copych/AcidBox/blob/main/media/acidjukebox2.mp3?raw=true) a bit of improvements

[demo3.mp3](https://github.com/copych/AcidBox/blob/main/media/acidjukebox3.mp3?raw=true) this time reverb and delay engaged (all onboard, all managed by AcidBanger), I have restarted it several times to show the variety of patterns generated.

[demo4.mp3](https://github.com/copych/AcidBox/blob/main/media/acidjukebox4.mp3?raw=true) fully automated breaks and fills.

# To build the thing
Ideally, you need an ESP32 with PSRAM (ESP32 WROVER module). Also an external DAC, like PCM5102 is highly preferrable. 

<b>ATTENTION! PSRAM handling requires up-to-date Arduino ESP32 core. Of what I tested, versions 2.0.4, 2.0.5, 2.0.6 are working, 2.0.0 is not</b>

In ArduinoIDE (I've used v.1.8.20) select: 
* board: ESP32 Dev Module
* partition scheme: No OTA (1MB APP/ 3MB SPIFFS)
* PSRAM: enabled

Also you will need to upload drum samples to the ESP32 flash (LittleFS). To do so follow the instructions: https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool

# What if you only have WROOM module (no PSRAM)
You can still compile and run the app, with NO_PSRAM option (line 11 in config.h). Note, that in this case you will get NO REVERB and just ONE SMALL DRUM KIT. 

In ArduinoIDE (I used v.1.8.20) select: 
* board: ESP32 Dev Module
* partition scheme: No OTA (1MB APP/ 3MB SPIFFS)
* PSRAM: disabled

Also you will need to upload drum samples to the ESP32 flash (LittleFS). To do so follow the instructions: https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool

# What if you don't have an external DAC module (PCM5102)
You can still compile and run the app, with USE_INTERNAL_DAC option (line 10 in config.h). BUT you should understand that sound output is 8-bit. You just get it form GPIO25 and GPIO26. Probably you can improve it a bit playing with the multipliers in i2s_output() method in general.ino file.

In ArduinoIDE (I used v.1.8.20) select:
* board: ESP32 Dev Module
* partition scheme: No OTA (1MB APP/ 3MB SPIFFS)

Also you will need to upload drum samples to the ESP32 flash (LittleFS). To do so follow the instructions: https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool



# Functional diagram 
<img src="https://github.com/copych/AcidBox/blob/main/media/2022-12-14_00-03-18.png" width=100%>
("Acid Banger" JukeBox actually calls midi functions as an external app)

# Thanks go to
* Marcel Licence https://github.com/marcel-licence : synth and DSP related code (delay, bi-filter, bitcrusher initially were taken from here)
* Infrasonic Audio https://github.com/infrasonicaudio : synth and DSP related code (moogladder filter, wavefolder and i2s base code were initially taken from here)
* Erich Heinemann https://github.com/ErichHeinemann : forked Marcel Licence's sampler and drum samples were taken from here
* Dimitri Diakopoulos https://github.com/ddiakopoulos/MoogLadders : a collection of c++ implementations of Moogladder filters
* Open303 project https://sourceforge.net/projects/open303/, https://www.kvraudio.com/forum/viewtopic.php?t=262829 -- guys have done a lot of research, their filters combination is now the default one.

<img src="https://github.com/copych/AcidBox/blob/main/media/2022-12-13%2015-44-53.JPG" width=100%>
