#ifndef FIREBASE_RTDB_CLIENT_H
#define FIREBASE_RTDB_CLIENT_H

#include <Arduino.h>

bool saveToFirebaseRTDB(
    int spo2, 
    int heartRate, 
    float temperature, 
    bool validSpO2, 
    bool validHR, 
    bool validTemp
);

#endif