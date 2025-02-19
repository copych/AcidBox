#include "DataBus.h"

DataBus::DataBus() {
  _numberOfBusListeners = 0;
}

void DataBus::init() {
}

void DataBus::loop() {
}

void DataBus::registerToBus(IBusListener *listener) {
  _busListeners[_numberOfBusListeners] = listener;
  _numberOfBusListeners++;
  // Addressing is 1 based to prevent messages arriving at address 0
  listener->setBusAddress(_numberOfBusListeners);
#ifdef DEBUG_DATABUS
  DEBF("Registered listener of type: %d\r\n", listener->getListenerReceiverType());
  DEBF("Number of listeners: %d\r\n"  ,_numberOfBusListeners); 
#endif
}

void DataBus::busMessage(Message message) {
  // If destination address is set directly sent message
  if(message.destinationAddress > 0) {
      _busListeners[message.destinationAddress - 1]->receiveBusMessage(message);
      return;
  }

  for(unsigned char i = 0; i < _numberOfBusListeners; i++) {
    // If the listener is subscribed to this message type, send the message
    if(_busListeners[i]->canReceiveMessage(message.command)) {
      _busListeners[i]->receiveBusMessage(message);
    }
  }  
}

void DataBus::sendMessage(unsigned char sourceAddress, unsigned char destinationAddress, Command command, ReceiverType receiverType) {
  Message message;
  message.sourceAddress = sourceAddress;
  message.destinationAddress = destinationAddress;
  message.command = command;
  message.receiverType = receiverType;
  this->busMessage(message);
}

void DataBus::sendDataMessage(unsigned char sourceAddress, unsigned char destinationAddress, Command command, ReceiverType receiverType, void *data) {
  Message message;
  message.sourceAddress = sourceAddress;
  message.destinationAddress = destinationAddress;
  message.command = command;
  message.receiverType = receiverType;
  message.data = data;
  this->busMessage(message);
}
