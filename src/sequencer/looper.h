#pragma once

#include <Arduino.h>

/*
This is the core sequencer module
this pattern looper is intended for live midi looping
*/

#undef min
#undef max
// use _min, _max or std::min and std::max instead

#include "looper_config.h"
#include "../databus/IBusListener.h"
#include "../databus/DataBus.h"
#include <vector>
#include <stdexcept>
#include "track.h"
#include "../general/general.h"

namespace performer {

typedef enum  { SEQ_PLAY,               // currently playing
                SEQ_PAUSE,              // SPP contains the number of the current 16th note of the loop
                SEQ_STOP,               // rewind to zero position: SPP = 0 
                NUM_SEQ_STATES 
} eSeqStates_t;

typedef enum  { SYNC_INT_MICROS,        // checking micros() in the loop()
                SYNC_INT_ISR,           // interrupt timer // not implemented yet (requires static class members and vars, and IRAM_ATTR, or move these routines to the global scope)
                                        // I wouldn't suffer much because of this lack, cause of what I have tested, ISR doesn't give that much advantage in stability
                                        // and if you aren't trying to put the project within the last few remaining CPU ticks, just don't you care
                SYNC_EXT_MTC,           // external midi time code // not implemented
                SYNC_EXT_TC,            // external midi timing clock // not implemented
                NUM_SYNC_MODES
} eSyncModes_t;  

// sequencer data
class Looper : public IBusListener {

public:
	Looper(DataBus *dataBus);
	int getBpm() 						{return _bpm;}
	int getLoopSteps() 					{return _loopSteps;}
	int getCurrentStep() 				{return _currentStep;}
	int getPpqn() 						{return _ppqn;}
	eSeqStates_t 	getSeqState()		{return _seqState;}
	eSyncModes_t 	getSyncMode()		{return _syncMode;}

	/** IBusListener methods */
	void receiveBusMessage(Message message);
	enum ReceiverType getListenerReceiverType();
	void busInit();

	int addTrack(eTrackType_t trackType, byte midiChannel); // adds a track, returns the index of a newly added track (not thread-safe)
	int addPattern(int trackNum); // adds a pattern to a track with id=trackNum, returns the index of a newly added pattern (not thread-safe)
	Track* getTrack(int trackNum);
	int getNumberOfTracks()				{return Tracks.size();}

	void setSyncOnOff(bool sendSync) 	{_sendSync = sendSync;}
	void setSeqState(eSeqStates_t new_state);
	void setSyncMode(eSyncModes_t new_mode);
	void setBpm(float new_bpm);
	void setLoopSteps(size_t new_steps);
	void setCurrentStep(size_t step_n);
	void setPpqn(int new_ppqn);
	inline void setMidiControlHandler(std::function<void(uint8_t, uint8_t, uint8_t)> cb)        {_cb_midi_control = cb;}
	inline void setMidiNoteOnHandler(std::function<void(uint8_t, uint8_t, uint8_t)> cb)         {_cb_midi_note_on = cb;}
	inline void setMidiNoteOffHandler(std::function<void(uint8_t, uint8_t, uint8_t)> cb)        {_cb_midi_note_off = cb;}
	inline void setMidiProgramChangeHandler(std::function<void(uint8_t, uint8_t)> cb)           {_cb_midi_program_change = cb;}
	inline void setMidiPitchBendHandler(std::function<void(uint8_t, int)> cb)                   {_cb_midi_pitchbend = cb;}
	void setSwing(float new_swing);			// 0.0 = straight, 0.66 = duplets, -0.66 = also duplets, but kinda irish
	void setGrid();						          // precalculate pulse grid according to swing value
	void onPulse();                     // fire on every pulse
	void onStep();                      // fire according to the pulse grid
	void onPreStep();                   // fire one pulse before the grid line
	void onPostStep();                   // fire one pulse after the grid line
	void stop();                        // stops (actually pauses) the playback
	void play();                        // reset and start from the zero position
	void resume();                      // continues playback
	void looperTask();                  // this is to call in loop(); ~1000Hz is quite enough, while 250Hz introduces at max +/-4ms quantization error, which may be audible
	
private:
	DataBus *_dataBus;
	std::vector <Track> Tracks;
	int				_ppqn 			= 24; 				  // MIDI sync pulses per quarter note 
	int				_q_ppqn			= _ppqn / 4;		// often appearing 16th length
	float			_bpm 			  = 130.0;
	size_t	  		_pulseMicros	= 60.0f * 1e6 / _ppqn / _bpm;
	float			_swing			  = 0.0f;				// -1.0 .. 1.0 making every odd 16th note to fire earlier or later up to the closest 32nd note
	int				_swingPulses	= 0;
	int       		_swingMicros  = 0;
	int       		_startMicros  = 0;
	int				_loopSteps 		= 16;				  // Here we asume that "step" = 16th note, and "swing" is a deviation of a straight rhythm 
	int				_currentStep 	= 0;
	int				_currentPulse = 0;				  // pulse counter
	int				_harmonyRoot 	= 0;				  // root note 0-11 for harmonizing
	int				_harmonyThird	= 3;				  // semitones: minor = 3, major = 4, sus2 = 2, sus4 = 5, or 0 if omitted
	int				_harmonyFifth 	= 7;				// semitones: dim = 6, clean V = 7, or 0 if omitted
	int				_harmonySeventh = 10;				// semitones: VII = 10, maj7 = 11, or 0 if omitted
	int				_harmonyAddon 	= 0;				// semitones: any number 1-11, will be shifted up by an octave
	bool			_sendSync 		  = false;		// send MIDI sync or not
	bool      		_sendControlsOnPreStep = false;
	bool      		_sendNoteOffsOnPreStep = false;
	bool      		_sendPortaAsOverlap = false;
	eSeqStates_t	_seqState 	= SEQ_STOP;
	eSyncModes_t	_syncMode 	= SYNC_INT_MICROS;
	int 			_num_tracks 	  = 0;
	size_t 			_pulseTriggers[MAX_LOOP_STEPS];
	size_t			_nextPulseTrigger;
	size_t			_nextPulseMicros;

	void handleNoteStackOnPulse();
	
	std::function<void(uint8_t, uint8_t, uint8_t)> _cb_midi_control;
	std::function<void(uint8_t, uint8_t, uint8_t)> _cb_midi_note_on;
	std::function<void(uint8_t, uint8_t, uint8_t)> _cb_midi_note_off;
	std::function<void(uint8_t, int)> _cb_midi_pitchbend;
	std::function<void(uint8_t, uint8_t)> _cb_midi_program_change;
 
};

} // namespace

