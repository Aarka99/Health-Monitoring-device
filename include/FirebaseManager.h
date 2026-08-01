#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Firebase_ESP_Client.h>
 
// Use pointers to reduce static memory
extern FirebaseData* fbdo;
extern FirebaseAuth* auth;
extern FirebaseConfig* config;

void initFirebase(const char* apiKey, const char* dbUrl, const char* projectId);

#endif