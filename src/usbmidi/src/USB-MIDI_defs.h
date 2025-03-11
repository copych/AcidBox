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

#include "USB-MIDI_Namespace.h"

BEGIN_USBMIDI_NAMESPACE

using namespace MIDI_NAMESPACE;

// from https://www.usb.org/sites/default/files/midi10.pdf
// 4 USB-MIDI Event Packets
// Table 4-1: Code Index Number Classifications

static uint8_t type2cin[][2] = { {MidiType::InvalidType,0}, {MidiType::NoteOff,8}, {MidiType::NoteOn,9}, {MidiType::AfterTouchPoly,0xA}, {MidiType::ControlChange,0xB}, {MidiType::ProgramChange,0xC}, {MidiType::AfterTouchChannel,0xD}, {MidiType::PitchBend,0xE} };

static uint8_t system2cin[][2] = { {MidiType::SystemExclusive,0}, {MidiType::TimeCodeQuarterFrame,2}, {MidiType::SongPosition,3}, {MidiType::SongSelect,2}, {0,0}, {0,0}, {MidiType::TuneRequest,5}, {MidiType::SystemExclusiveEnd,0}, {MidiType::Clock,0xF}, {0,0}, {MidiType::Start,0xF}, {MidiType::Continue,0xF}, {MidiType::Stop,0xF}, {0,0}, {MidiType::ActiveSensing,0xF}, {MidiType::SystemReset,0xF} };

static byte cin2Len[][2] = { {0,0}, {1,0}, {2,2}, {3,3}, {4,0}, {5,0}, {6,0}, {7,0}, {8,3}, {9,3}, {10,3}, {11,3}, {12,2}, {13,2}, {14,3}, {15,1} };

#define GETCABLENUMBER(packet) (packet.header >> 4);
#define GETCIN(packet) (packet.header & 0x0f);
#define MAKEHEADER(cn, cin) (((cn & 0x0f) << 4) | cin)
#define RXBUFFER_PUSHBACK1 { mRxBuffer[cn][mRxLength[cn]++] = mPacket.byte1; }
#define RXBUFFER_PUSHBACK2 { mRxBuffer[cn][mRxLength[cn]++] = mPacket.byte1; mRxBuffer[cn][mRxLength[cn]++] = mPacket.byte2; }
#define RXBUFFER_PUSHBACK3 { mRxBuffer[cn][mRxLength[cn]++] = mPacket.byte1; mRxBuffer[cn][mRxLength[cn]++] = mPacket.byte2; mRxBuffer[cn][mRxLength[cn]++] = mPacket.byte3; }

#define RXBUFFER_POPFRONT(byte) auto byte = mRxBuffer[cableNumber][mRxIndex[cableNumber]++]; mRxLength[cableNumber]--;
#define SENDMIDI(packet) { MidiUSB.sendMIDI(packet); MidiUSB.flush(); }

END_USBMIDI_NAMESPACE
