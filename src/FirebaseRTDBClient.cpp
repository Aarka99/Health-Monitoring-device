#include "FirebaseRTDBClient.h"
#include "FirebaseManager.h"
#include <Arduino.h>

extern String activePatientUID;
bool saveToFirebaseRTDB(
    int spo2, int heartRate, float temperature,
    bool validSpO2, bool validHR, bool validTemp)
{
    if (!Firebase.ready() || !fbdo)
        return false;

    bool success = true;

    char path[120];

    if (validSpO2)
    {

        sprintf(path,
                "/live_data/%s/spo2",
                activePatientUID.c_str());

        if (!Firebase.RTDB.setInt(fbdo, path, spo2))
        {
            Serial.println("SpO2 failed");
            success = false;
        }
    }

    if (validHR)
    {

        sprintf(path,
                "/live_data/%s/heart_rate",
                activePatientUID.c_str());

        if (!Firebase.RTDB.setInt(fbdo, path, heartRate))
        {
            Serial.println("HR failed");
            success = false;
        }
    }

    if (validTemp)
    {

        sprintf(path,
                "/live_data/%s/temperature",
                activePatientUID.c_str());

        if (!Firebase.RTDB.setFloat(fbdo, path, temperature))
        {
            Serial.println("Temp failed");
            success = false;
        }
    }
    return success;
}