#include "looper.h"
#include "misc.h"
#include <math.h>

using namespace performer;

void Looper::looperTask() {
  if (_seqState == SEQ_PLAY) {
  	switch (_syncMode) {
  		case SYNC_INT_MICROS:
  			if (micros() >= _nextPulseMicros) {
  				_nextPulseMicros = micros() + _pulseMicros;
  				onPulse();
  			}
  			break;
  		case SYNC_INT_ISR: // just in case
  			onPulse();
  			break;
  		case SYNC_EXT_MTC:
  		case SYNC_EXT_TC:
  		default:
  			break;
  	}
  }
}

void Looper::setBpm(float new_bpm){
	_bpm = new_bpm;
	_pulseMicros = 60.0f * 1e6 / _ppqn / _bpm;
	_q_ppqn = _ppqn / 4;
	_swingPulses = round(_swing * (_q_ppqn / 2) );
	_swingMicros = _swingPulses * _pulseMicros; 			// microseconds to shift odd 16th notes
 	setGrid();
}

void Looper::setLoopSteps(size_t new_steps){
	new_steps = constrain(new_steps, 1, MAX_LOOP_STEPS);
	_loopSteps = new_steps;
}

void Looper::setCurrentStep(size_t step_n){
	if (step_n < _loopSteps) {
		_currentStep = step_n;
		_currentPulse = _pulseTriggers[step_n]; 
		setGrid();
	}
}

void Looper::setPpqn(int new_ppqn){
	_ppqn = new_ppqn;
	setGrid();
}

void Looper::setSwing(float new_swing) {
	fclamp(new_swing, -1.0, 1.0);
	_swing = new_swing;
	_swingPulses = round( new_swing * (_q_ppqn / 2) );
	_swingMicros = _swingPulses * _pulseMicros; 			// microseconds to shift odd 16th notes
	setGrid();
}

void Looper::setGrid() {
	for (int i = 0; i < MAX_LOOP_STEPS; ++i) {
		// calc moments to fire sequencer events
		if (i % 2 == 0) {
			_pulseTriggers[i] = i * _q_ppqn;
		} else {
			_pulseTriggers[i] = i * _q_ppqn + _swingPulses;
		}
 //   DEB(_pulseTriggers[i]);
 //   DEB(", ");
	}
 DEBUG();
}

int Looper::addTrack(eTrackType_t track_type, byte midi_channel){
  Tracks.emplace_back(Track(track_type, midi_channel));
  return (Tracks.size()-1);
}

void Looper::onPulse(){
	if (_currentPulse ==_nextPulseTrigger){
		onStep();
	} else if ((_currentPulse ==_nextPulseTrigger-1) || (_currentPulse ==_nextPulseTrigger+(_loopSteps * _q_ppqn)-1)){
  //  onPreStep();
  } else if ((_currentPulse ==_nextPulseTrigger+1) || (_currentPulse ==_nextPulseTrigger-(_loopSteps * _q_ppqn)+1)){
 //   onPostStep();
  }
	_currentPulse = (_currentPulse + 1) % (_loopSteps * _q_ppqn);
}
/*
void Looper::onPreStep(){
  int preStep = (_currentStep + 1 ) % _loopSteps;
  for ( auto &tr: Tracks) {
    for ( auto &patt: tr.Patterns) {
#if defined MIDI_VIA_SERIAL || defined MIDI_VIA_SERIAL2
        for ( auto &st: patt.Steps[preStep]) { // send controls or note-offs first
          switch (st.type) {
            case EVT_CONTROL_CHANGE:
              if (_sendControlsOnPreStep) {
                MIDI.sendControlChange ( st.value1 , st.value2 , tr.getMidiChannel() );
              }
              break;
            case EVT_NOTE_OFF:
              if (_sendNoteOffsOnPreStep) {
                MIDI.sendNoteOff ( st.value1 ,  0,  tr.getMidiChannel() );
              }
              break;
            case EVT_NOTE_ON:
              // terminate current sounding note
              if (_sendNoteOffsOnPreStep) {
                if (tr.getTrackType() == TRACK_MONO) MIDI.sendNoteOff ( tr.getPrevNote() , 0, tr.getMidiChannel() );
              }
              break;
          }
        }
#endif
    }
  }
  // to be sure that we first send controller data and note-OFFs, and then send note-ONs
 // DEBF("preStep for %d \r\n", _currentStep);
}



void Looper::onPostStep(){
  if (_sendPortaAsOverlap) {
    for ( auto &tr: Tracks) {
      for ( auto &patt: tr.Patterns) {
        for ( auto &st: patt.Steps[_currentStep]) { // send controls or note-offs first
          switch (st.type) {
            case EVT_CONTROL_CHANGE:
              if (st.value1 == CC_PORTAMENTO && st.value2 > 63) { // slide on
                MIDI.sendControlChange ( st.value1 , st.value2 , tr.getMidiChannel() );
              }
              break;
          }
        }
      }
    }
    for ( auto &tr: Tracks) {
      for ( auto &patt: tr.Patterns) {
  #if defined MIDI_VIA_SERIAL || defined MIDI_VIA_SERIAL2
          for ( auto &st: patt.Steps[_currentStep]) { // send controls or note-offs first
            switch (st.type) {
              case EVT_CONTROL_CHANGE:
                if (_sendControlsOnPreStep) {
                  MIDI.sendControlChange ( st.value1 , st.value2 , tr.getMidiChannel() );
                }
                break;
              case EVT_NOTE_OFF:
                if (_sendNoteOffsOnPreStep) {
                  MIDI.sendNoteOff ( st.value1 ,  0,  tr.getMidiChannel() );
                }
                break;
              case EVT_NOTE_ON:
                // terminate current sounding note
                if (_sendNoteOffsOnPreStep) {
                  if (tr.getTrackType() == TRACK_MONO) MIDI.sendNoteOff ( tr.getPrevNote() , 0, tr.getMidiChannel() );
                }
                break;
            }
          }
  #endif
      }
    }
  }
}
*/
void Looper::onStep(){
  _currentStep++;
	_currentStep = _currentStep % _loopSteps;
	_nextPulseTrigger = _pulseTriggers[(_currentStep + 1 ) % _loopSteps];
  for ( auto &tr: Tracks) {
    uint8_t prev_note = tr.getPrevNote();
    for ( auto &patt: tr.Patterns) {
      bool slide = patt.isSlide(_currentStep);
      for ( auto &st: patt.Steps[_currentStep]) { // send controls first
          switch (st.type) {
            case EVT_CONTROL_CHANGE:
              if (!_sendControlsOnPreStep) {
                _cb_midi_control(tr.getMidiChannel() , st.value1 , st.value2  );
              }
              break;
            case EVT_NOTE_OFF:
              if (!_sendNoteOffsOnPreStep) {
                _cb_midi_note_off(tr.getMidiChannel() , st.value1 ,  0);
              }
              break;
          }
        }
      for ( auto &st: patt.Steps[_currentStep]) { // send notes afterwards
        switch (st.type) {
          case EVT_NOTE_ON:
            if ( (!_sendControlsOnPreStep) && (tr.getTrackType() == TRACK_MONO) && !slide ) {
              _cb_midi_note_off(tr.getMidiChannel(), prev_note , 0  );
            }
            _cb_midi_note_on(tr.getMidiChannel(), st.value1  ,  st.value2 );
            if ( (!_sendControlsOnPreStep) && (tr.getTrackType() == TRACK_MONO) && slide ) {
              _cb_midi_note_off(tr.getMidiChannel(),  prev_note , 0 );
            }
            tr.setPrevNote(st.value1);
            break;
        }
        DEBF("step: %d, chan: %d, evt: %s, v1: %d, v2: %d \r\n", _currentStep, tr.getMidiChannel(), str_events[st.type], st.value1, st.value2);
      }
    }
  }
}

void Looper::stop(){
	_seqState = SEQ_STOP;
}

void Looper::play(){
	_startMicros = micros();
  _currentPulse = 0;
  _currentStep = _loopSteps-1;
  _nextPulseTrigger = 0;
  setGrid();
	_seqState = SEQ_PLAY;
}

void Looper::resume(){
	_seqState = SEQ_PLAY;
}
