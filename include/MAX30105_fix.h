// MAX30105_fix.h
#ifdef I2C_BUFFER_LENGTH
    #undef I2C_BUFFER_LENGTH
#endif

#include "MAX30105.h"

#ifndef I2C_BUFFER_LENGTH
    #define I2C_BUFFER_LENGTH 128
#endif