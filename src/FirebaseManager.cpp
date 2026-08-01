#include "FirebaseManager.h"
#include <Arduino.h>

// Reduce global objects - use pointers
FirebaseData *fbdo = nullptr;
FirebaseAuth *auth = nullptr;
FirebaseConfig *config = nullptr;
void initFirebase(const char* apiKey, const char* dbUrl, const char* projectId) {
    Serial.println(F("Initializing Firebase..."));
    
    // Allocate only when needed
    if (!config) {
        config = new FirebaseConfig();
        auth = new FirebaseAuth();
        fbdo = new FirebaseData();
    }
    
    config->api_key = apiKey;
    config->database_url = dbUrl;
    
    auth->user.email = "rkproject25@gmail.com";
    auth->user.password = "rajesh";
    
    Firebase.begin(config, auth);
    Firebase.reconnectWiFi(true);
    
    // Wait for connection
    int attempts = 0;
    while (!Firebase.ready() && attempts < 10) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (Firebase.ready()) {
        Serial.println(F("\nFirebase connected!"));
    } else {
        Serial.println(F("\nFirebase connection failed!"));
    }
}