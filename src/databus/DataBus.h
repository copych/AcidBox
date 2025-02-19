#ifndef DATA_BUS_H
#define DATA_BUS_H

#include "Databus_shared.h"
#include "IBusListener.h"
#include "../../config.h"

class DataBus {

    public:
      DataBus();
      void init();
      void loop();
      void registerToBus(IBusListener *listener);
      void busMessage(Message message);
      void sendMessage(unsigned char sourceAddress, unsigned char destinationAddress, Command command, ReceiverType receiverType);
      void sendDataMessage(unsigned char sourceAddress, unsigned char destinationAddress, Command command, ReceiverType receiverType, void *data);

    private:
      unsigned char _numberOfBusListeners;
      // TODO, now starting with 10 pointers, check if this is what we need
      IBusListener *_busListeners[10];


};
#endif