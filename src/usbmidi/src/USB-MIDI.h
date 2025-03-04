/*
 Copyright (c) 2020 lathoub

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#pragma once

#include <MIDI.h>
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#if defined(ESP32_USB_HOST)
#include <MIDIUSBHOST_ESP32.h>
#else
//#include <MIDIUSB_ESP32.h>
#include "src/midiusb/src/MIDIUSB_ESP32.h"
#endif
#else
#include "MIDIUSB.h"
#define USB_MIDI_NUM_CABLES 1
#if USB_MIDI_NUM_CABLES != 1
#error "Currently MIDIUSB Library for Arduino does not support Multi Cable"
#endif
#endif

#include "USB-MIDI_defs.h"
#include "USB-MIDI_Namespace.h"

BEGIN_USBMIDI_NAMESPACE

class usbMidiTransport
{
private:
    byte mTxBuffer[4];
    size_t mTxIndex;
    MidiType mTxStatus;

    static byte mRxBuffer[USB_MIDI_NUM_CABLES][64];
    static size_t mRxLength[USB_MIDI_NUM_CABLES];
    static size_t mRxIndex[USB_MIDI_NUM_CABLES];

    midiEventPacket_t mPacket;
    uint8_t cableNumber;
    static uint8_t cableNumberTotal; 
    static uint16_t cableNumberMask; 

public:
    ~usbMidiTransport() {

    }
    usbMidiTransport(uint8_t cableNumber = 0)
	{
        if  (cableNumber >= USB_MIDI_NUM_CABLES || cableNumberMask & (1 << cableNumber)) {
            // Selected cable number is invalid
            this->cableNumber = 0xFF; 
        }
        this->cableNumber = cableNumber;
        cableNumberTotal++; 
        cableNumberMask |= (1 << cableNumber); 
	};

public:
    
    static const bool thruActivated = false;

	void begin()
	{
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
        MidiUSB.begin();
#endif
        mTxIndex = 0;
        mRxIndex[cableNumber] = 0;
        mRxLength[cableNumber] = 0;
    };

    void end()
    {
    }

	bool beginTransmission(MidiType status)
	{
        mTxStatus = status;
        
        byte cin = 0;
        if (status < SystemExclusive) {
            // Non System messages
            cin = type2cin[((status & 0xF0) >> 4) - 7][1];
            mPacket.header = MAKEHEADER(cableNumber, cin);
        }
        else {
            // Only System messages
            cin = system2cin[status & 0x0F][1];
            mPacket.header = MAKEHEADER(cableNumber, cin);
        }
        
        mPacket.byte1 = mPacket.byte2 = mPacket.byte3  = 0;
        mTxIndex = 0;

        return true;
	};

	void write(byte byte)
	{
        if (mTxStatus != MidiType::SystemExclusive) {
            if (mTxIndex == 0)      mPacket.byte1 = byte;
            else if (mTxIndex == 1) mPacket.byte2 = byte;
            else if (mTxIndex == 2) mPacket.byte3 = byte;
        }
        else if (byte == MidiType::SystemExclusiveStart) {
            mPacket.header = MAKEHEADER(cableNumber, 0x04);
            mPacket.byte1 = byte;
        }
        else // SystemExclusiveEnd or SysEx data
        {
            auto i = mTxIndex % 3;
            if (byte == MidiType::SystemExclusiveEnd)
                mPacket.header = MAKEHEADER(cableNumber, (0x05 + i));
            
            if (i == 0) {
                mPacket.byte1 = byte; mPacket.byte2 = mPacket.byte3 = 0x00;
            }
            else if (i == 1) {
                mPacket.byte2 = byte; mPacket.byte3 = 0x00;
            }
            else if (i == 2) {
                mPacket.byte3 = byte;
                if (byte != MidiType::SystemExclusiveEnd)
                    SENDMIDI(mPacket);
            }
        }
        mTxIndex++;
    };

	void endTransmission()
	{
        SENDMIDI(mPacket);
	};

	byte read()
	{
        RXBUFFER_POPFRONT(byte);
		return byte;
	};

	unsigned available()
	{
        // consume mRxBuffer first, before getting a new packet
        bool allCableAvailable = true; 
        for (uint8_t i = 0; i < USB_MIDI_NUM_CABLES; i++) {
            if (mRxLength[i] > 0) {
                allCableAvailable = false; 
                break; 
            }
                
        }

        if(!allCableAvailable) {
            return mRxLength[cableNumber];
        }
        
        mPacket = MidiUSB.read();
        if (mPacket.header != 0) {
            auto cn  = GETCABLENUMBER(mPacket);
            if (cn >= cableNumberTotal)
                return 0;

            mRxIndex[cn] = 0; 
            auto cin = GETCIN(mPacket);
            auto len = cin2Len[cin][1];
            switch (len) {
                case 0:
                    if (cin == 0x4 || cin == 0x7)
                        RXBUFFER_PUSHBACK3
                    else if (cin == 0x5)
                        RXBUFFER_PUSHBACK1
                    else if (cin == 0x6)
                        RXBUFFER_PUSHBACK2
                    break;
                case 1:
                    RXBUFFER_PUSHBACK1
                    break;
                case 2:
                    RXBUFFER_PUSHBACK2
                    break;
                case 3:
                    RXBUFFER_PUSHBACK3
                    break;
                default:
                    break; // error
            }
        }

        return mRxLength[cableNumber];
	};
};

byte usbMidiTransport::mRxBuffer[USB_MIDI_NUM_CABLES][64] = {0};
size_t usbMidiTransport::mRxLength[USB_MIDI_NUM_CABLES] = {0};
size_t usbMidiTransport::mRxIndex[USB_MIDI_NUM_CABLES] = {0};
uint8_t usbMidiTransport::cableNumberTotal = 0; 
uint16_t usbMidiTransport::cableNumberMask = 0; 

END_USBMIDI_NAMESPACE

/*! \brief
 */
#define USBMIDI_CREATE_INSTANCE(CableNr, Name)  \
    USBMIDI_NAMESPACE::usbMidiTransport __usb##Name(CableNr);\
    MIDI_NAMESPACE::MidiInterface<USBMIDI_NAMESPACE::usbMidiTransport> Name((USBMIDI_NAMESPACE::usbMidiTransport&)__usb##Name);

#define USBMIDI_CREATE_CUSTOM_INSTANCE(CableNr, Name, Settings)  \
    USBMIDI_NAMESPACE::usbMidiTransport __usb##Name(CableNr);\
    MIDI_NAMESPACE::MidiInterface<USBMIDI_NAMESPACE::usbMidiTransport, Settings> Name((USBMIDI_NAMESPACE::usbMidiTransport&)__usb##Name);

#define USBMIDI_CREATE_DEFAULT_INSTANCE()  \
    USBMIDI_CREATE_INSTANCE(0, MIDI)
