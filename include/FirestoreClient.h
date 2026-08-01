#ifndef FIRESTORE_CLIENT_H
#define FIRESTORE_CLIENT_H

#include <Arduino.h>

void firestoreInit(const char* apiKey, const char* projectId);
bool saveToFirestore(
    int spo2, 
    int heartRate, 
    float temperature, 
    bool validSpO2, 
    bool validHR, 
    bool validTemp
);

#endif