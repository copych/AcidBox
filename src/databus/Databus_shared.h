#ifndef DATABUS_SHARED_H
#define DATABUS_SHARED_H

typedef enum ReceiverType {
  UI,
  MIDIREC,
  ENCODER,
  SEQUENCER,
  CLOCK,
  ALL
} ReceiverType;

typedef enum Command {
  SEQUENCER_START_STOP = 1,
} Command;

struct  {
  ReceiverType receiverType;
  unsigned char sourceAddress = 0;
  unsigned char destinationAddress = 0;
  Command command;
  void *data;
} typedef Message;

#endif