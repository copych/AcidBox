#ifndef IBUS_LISTENER_H
#define IBUS_LISTENER_H

#include "Databus_shared.h"

class IBusListener {

    public:
        virtual void receiveBusMessage(Message message);
        virtual enum ReceiverType getListenerReceiverType();
        virtual void busInit();
        
        void setBusAddress(unsigned char busAddressToSet) {
            busAddress = busAddressToSet;
        }

        void setMessageSubscriptions(int messageSubscriptions) {
            this->messageSubscriptions = messageSubscriptions;
        }

        bool canReceiveMessage(Command command) {
            int canReceive = command & messageSubscriptions;
            return canReceive > 0;
        }

    protected:
        int messageSubscriptions = 0;
        unsigned char busAddress;
    
    private:
        

};
#endif