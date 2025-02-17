#ifndef SERIAL_COM
#define SERIAL_COM

#include<Arduino.h>

#include "settings.h"

void initializeSerial();

class SerialCom {

  public:
    static void initialize(unsigned const int timeout = Timing::SERIAL_PORT_INIT_TIMEOUT_MS);
    static void send(String payload);
    static String receiveProvisiongSettings();

};

#endif