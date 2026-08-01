#include "FirestoreClient.h"
#include "FirebaseManager.h"
#include <Arduino.h>

static String g_projectId;
extern String activePatientUID;
void firestoreInit(const char *apiKey, const char *projectId) {
    g_projectId = projectId;
}
bool saveToFirestore(
    int spo2, int heartRate, float temperature,
    bool validSpO2, bool validHR, bool validTemp
) {
    if (!Firebase.ready() || !fbdo) return false;

    FirebaseJson content;
    FirebaseJson fields;

    if (validSpO2) {
        FirebaseJson f;
        f.set("integerValue", spo2);
        fields.set("spo2", f);
    }

    if (validHR) {
        FirebaseJson f;
        f.set("integerValue", heartRate);
        fields.set("heart_rate", f);
    }

    if (validTemp) {
        FirebaseJson f;
        f.set("doubleValue", temperature);
        fields.set("temperature", f);
    }

   time_t now = time(nullptr);
struct tm *t = gmtime(&now);

char isoTime[30];

sprintf(isoTime,
"%04d-%02d-%02dT%02d:%02d:%02dZ",
t->tm_year+1900,
t->tm_mon+1,
t->tm_mday,
t->tm_hour,
t->tm_min,
t->tm_sec);

FirebaseJson tsField;
tsField.set("timestampValue", isoTime);

fields.set("timestamp", tsField);
    content.set("fields", fields);

    String path = "users/";
    path += activePatientUID;
    path += "/history";

    if (Firebase.Firestore.createDocument(
            fbdo,
            g_projectId.c_str(),
            "",
            path.c_str(),
            content.raw()))
    {
        Serial.println("Firestore Saved");
        return true;
    }

    Serial.println("Firestore Failed:");
    Serial.println(fbdo->errorReason());

    return false;
}