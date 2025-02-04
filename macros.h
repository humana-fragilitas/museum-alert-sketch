#ifndef MACROS_H
#define MACROS_H

  // Uncomment/comment the following line to enable debugging mode
  #define DEBUG 

  #ifdef DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
  #else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
  #endif

#endif  // MACROS_H