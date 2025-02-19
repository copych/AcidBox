#include "looper.h"
#include "misc.h"
#include <math.h>

using namespace performer;

Looper::Looper(DataBus *dataBus) {
		_dataBus = dataBus;
		messageSubscriptions = SEQUENCER_START_STOP;
}

void Looper::receiveBusMessage(Message message) {
  switch (message.command)
  {
  case SEQUENCER_START_STOP:
    if (_seqState == SEQ_PLAY) {
      stop();
    } else {
      play();
    }
    break;
  
  default:
    break;
  }
}

enum ReceiverType Looper::getListenerReceiverType() {
  return SEQUENCER;
}

void Looper::busInit() {

}

Track* Looper::getTrack(int trackNum) {
  if(Tracks.size() > trackNum) {
    return &Tracks[trackNum];
  }
  throw std::invalid_argument("Tracks vector out of bounds");
}


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
#ifdef DEBUG_SEQUENCER  
  DEBF("_bpm: %f, _pulseMicros: %d, _ppqnL: %d, _q_ppqn: %d, _swingPulses: %d, _swingMicros: %d \r\n", _bpm, _pulseMicros, _ppqn, _q_ppqn, _swingPulses, _swingMicros);
#endif
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
	General::fclamp(new_swing, -1.0, 1.0);
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
	}
}

int Looper::addTrack(eTrackType_t track_type, byte midi_channel){
  Tracks.emplace_back(Track(track_type, midi_channel));
  return (Tracks.size()-1);
}

void Looper::onPulse() {
  // Handle the noteStacks, if the note needs to go off, this method will send a note off.
  handleNoteStackOnPulse();  
	if (_currentPulse ==_nextPulseTrigger){
		onStep();
	}
	_currentPulse = (_currentPulse + 1) % (_loopSteps * _q_ppqn);
}

void Looper::handleNoteStackOnPulse() {
  for (auto &track : Tracks) {
    for(unsigned char i = 0; i < NOTE_STACK_SIZE; i++) {
      if (track._noteStack[i].length != -1 ) {
        --track._noteStack[i].length;
        if(track._noteStack[i].length == 0) {
          _cb_midi_note_off(track.getMidiChannel(), track._noteStack[i].note, 0);
          track._noteStack[i].length = -1;        
        }
      }
    }
  }
}

void Looper::onStep() {  
  _currentStep++;
	_currentStep = _currentStep % _loopSteps;
	_nextPulseTrigger = _pulseTriggers[(_currentStep + 1 ) % _loopSteps];
#ifdef DEBUG_SEQUENCER  
  DEBF("Step: %d\r\n", _currentStep);  
#endif
  for ( auto &tr: Tracks) {
#ifdef DEBUG_SEQUENCER
    DEBF("Track midi: %d\r\n", tr.getMidiChannel());
#endif     
    for (auto &patt: *tr.getPatterns()) {
      // for ( auto &st: patt.Steps[_currentStep]) { // send controls first
      //     switch (st.type) {
      //       case EVT_CONTROL_CHANGE:              
      //         if (!_sendControlsOnPreStep) {
      //           //_cb_midi_control(tr.getMidiChannel() , st.value1 , st.value2  );
      //         }
      //         break;
      //       case EVT_NOTE_OFF:                 
      //         if (!_sendNoteOffsOnPreStep) {
      //           //_cb_midi_note_off(tr.getMidiChannel() , st.value1 ,  0);
      //         }
      //         break;
      //     }
      //   }      
        // for ( auto &st: patt.Steps[_currentStep]) { // send notes afterwards
        //   switch (st.type) {
        //     case EVT_NOTE_ON:            
        //       // // Add the note to the stack to trigger the note off
        //       // if(tr.addStackNote(st.value1, st.length)) {
        //       //   _cb_midi_note_on(tr.getMidiChannel(), st.value1, st.value2 );
        //       // } else {
        //       //   #ifdef DEBUG_SEQUENCER
        //       //       DEB("Note stack full\r\n");
        //       //   #endif  
        //       // }
        //       // tr.setPrevNote(st.value1);
        //       break;
        //   }
        // }

        
        std::vector<sStepEvent_t> *noteEvents = patt.getNotes(_currentStep);        
        for (auto &noteEvent: *noteEvents) {
          if(tr.addStackNote(noteEvent.value1, noteEvent.length)) {
            _cb_midi_note_on(tr.getMidiChannel(), noteEvent.value1, noteEvent.value2 );
          } else {
            #ifdef DEBUG_SEQUENCER
                        DEB("Note stack full\r\n");
            #endif  
          }
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
