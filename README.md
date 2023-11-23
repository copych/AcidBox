# AcidBox
ESP32 headless acid combo of tb303 + tb303 + tr808 like synths. Filter cutoff, reso, env mod, accent, wavefolder, overdrive within each 303, per-instrument tunings, hi-pass/lo-pass filter and bitcrusher in drums, send to reverb, delay and master compression.
All MIDI driven. 44100, 16bit stereo I2S output to the external DAC or 8bit to the built-in DAC. No indication. Uses both cores of ESP32. Cheap ~$10.
Consult with midi_config.h to find out and to set up MIDI continous control messages.

# It can be a JukeBox
If you compile with #define JUKEBOX option in config.h, this becomes a stand-alone ACID-JUKE-BOX. You just listen.
A modified version of http://tips.ibawizard.net/acid-banger/, initially taken from https://github.com/vitling/acid-banger included, there's no AI, but randomizing pattern algorithms.

# YouTube Video
[![Video](https://img.youtube.com/vi/mhCWuZB_Tos/maxresdefault.jpg)](https://www.youtube.com/watch?v=mhCWuZB_Tos)

# MP3 Sound samples
[demo4.mp3](https://github.com/copych/AcidBox/blob/main/media/acidjukebox4.mp3?raw=true) fully automated breaks and fills.

[demo5.mp3](https://github.com/copych/AcidBox/blob/main/media/acidjukebox5.mp3?raw=true) 7 minutes of random fully automated acid.

# To build the thing
Ideally, you need an ESP32 or ESP32S3 with at least 4MB PSRAM (for example, ESP32 WROVER module). Also an external DAC, like PCM5102 is highly preferrable. 

<b>ATTENTION! PSRAM handling requires up-to-date Arduino ESP32 core. Of what I have tested, versions 2.0.6 up to current 2.0.14 are working with both ESP32 and ESP32S3, while 2.0.0 is not working</b>

In ArduinoIDE (I've used v.1.8.20) select: 
* board: ESP32 Dev Module (or ESP32S3 Dev Module if you use S3)
* partition scheme: No OTA (1MB APP/ 3MB SPIFFS)
* PSRAM: enabled (or the type of your PSRAM)

Also you will need to upload drum samples to the ESP32 flash (LittleFS). To do so follow the instructions: https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool

If you don't upload samples, the app will use the default drum kit from the samples.h

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


# MIDI Control
For the time being the following list of MIDI continious controllers is available:

    #define CC_303_PORTATIME    5   // affects gliding time
    #define CC_303_VOLUME       7   // mix volume
    #define CC_303_PORTAMENTO   65  // gliding on/off
    #define CC_303_PAN          10  // pano
    #define CC_303_WAVEFORM     70  // Blend between square and saw
    #define CC_303_RESO         71
    #define CC_303_CUTOFF       74
    #define CC_303_ATTACK       73
    #define CC_303_DECAY        72
    #define CC_303_ENVMOD_LVL   75
    #define CC_303_ACCENT_LVL   76
    #define CC_303_REVERB_SEND  91
    #define CC_303_DELAY_SEND   92
    #define CC_303_DISTORTION   94
    #define CC_303_OVERDRIVE    95

    // 808 Drums MIDI CC
    #define CC_808_VOLUME       7
    #define CC_808_PAN          10
    #define CC_808_RESO         71
    #define CC_808_CUTOFF       74  // Note that this filter's behaviour differs from the 303's, 64-127 means HP-, and 0-63 LP-filtering. 'Untouched' is at ~64.  
    #define CC_808_REVERB_SEND  91
    #define CC_808_DELAY_SEND   92
    #define CC_808_DISTORTION   94  // BitCrusher
    #define CC_808_BD_TONE      21  // Bass Drum tone control
    #define CC_808_BD_DECAY     23  // Bass Drum envelope decay
    #define CC_808_BD_LEVEL     24  // Bass Drum mix level
    #define CC_808_SD_TONE      25  // Snare Drum tone control
    #define CC_808_SD_SNAP      26  // Snare Drum envelope decay
    #define CC_808_SD_LEVEL     29  // Snare Drum mix level
    #define CC_808_CH_TUNE      61  // Closed Hat tone control
    #define CC_808_CH_LEVEL     63  // Closed Hat mix level
    #define CC_808_OH_TUNE      80  // Open Hat tone control
    #define CC_808_OH_DECAY     81  // Open Hat envelope decay
    #define CC_808_OH_LEVEL     82  // Open Hat mix level

    // Global 
    #define CC_ANY_COMPRESSOR   93
    #define CC_ANY_DELAY_TIME   84  // delay time
    #define CC_ANY_DELAY_FB     85  // delay feedback level
    #define CC_ANY_DELAY_LVL    86  // delay mix level
    #define CC_ANY_REVERB_TIME  87  // rebverb time
    #define CC_ANY_REVERB_LVL   88  // reverb mix level
    #define CC_ANY_RESET_CCS    121
    #define CC_ANY_NOTES_OFF    123
    #define CC_ANY_SOUND_OFF    120

# Functional diagram 
<img src="https://github.com/copych/AcidBox/blob/main/media/2022-12-14_00-03-18.png" width=100%>
("Acid Banger" JukeBox actually calls midi functions as an external app would do)

# Schematics and PCB
[@streetuff](https://github.com/streetuff) has made such a great PCB and schematics! https://github.com/streetuff/AcidBox-PCB
[![Schematics and PCB](https://github.com/streetuff/AcidBox-PCB/blob/main/3d-model.png)](https://github.com/streetuff/AcidBox-PCB)

# Thanks go to
* Marcel Licence https://github.com/marcel-licence : synth and DSP related code (delay, bi-filter, bitcrusher initially were taken from here)
* Infrasonic Audio https://github.com/infrasonicaudio : i2s base code was initially taken from here
* Seeduino Electro-Smith https://github.com/electro-smith/DaisySP : a lot of DSP and synth related code here 
* Erich Heinemann https://github.com/ErichHeinemann : forked Marcel Licence's sampler and initial set of drum samples were taken from here
* Dimitri Diakopoulos https://github.com/ddiakopoulos/MoogLadders : a collection of c++ implementations of Moogladder filters
* Open303 project https://sourceforge.net/projects/open303/, https://www.kvraudio.com/forum/viewtopic.php?t=262829 -- guys have done a lot of research, their filters combination is now the default one.

# 
<img src="https://github.com/copych/AcidBox/blob/main/media/2022-12-13%2015-44-53.JPG" width=100% > ESP32 proto


<img src="https://github.com/copych/AcidBox/blob/main/hardware/2023-03-28%20at%2009.19.49.jpeg" width=100% > ESP32s3 proto
