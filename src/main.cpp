#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <math.h>
#include <time.h>
#include "MAX30105_fix.h"
#include "FirebaseManager.h"
#include "FirebaseRTDBClient.h"
#include "FirestoreClient.h"

#define SAMPLE_BUFFER_SIZE 100
#define HISTORY_SIZE 3

#define WIFI_SSID "projectwifi"
#define WIFI_PASSWORD "source.pyc"

#define FIREBASE_API_KEY "AIzaSyAQyMyg9oV6s91WPmoruNfavdZjl9Fg6SY"
#define FIREBASE_DB_URL "https://kidroute-fc02f-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIRESTORE_PROJECT "kidroute-fc02f"

#define DEVICE_ID "3OqE6xVPKAfyjvOFgrSNlXlodBw2"

#define BUZZER_PIN 25
#define Thermester_PIN 34

#define SPO2_MIN 80
#define SPO2_MAX 100
#define HR_MIN 40
#define HR_MAX 160

MAX30105 particleSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);

uint32_t irBuffer[SAMPLE_BUFFER_SIZE];
uint32_t redBuffer[SAMPLE_BUFFER_SIZE];

String activePatientUID(DEVICE_ID);
void fetchActivePatientUID();
struct SensorData
{
    int32_t spo2;
    int32_t heartRate;
    float temperature;
    bool validSPO2;
    bool validHR;
    bool validTemp;
} g_data;

TaskHandle_t SensorTaskHandle;
TaskHandle_t CloudTaskHandle;
uint32_t irValueGlobal = 0;
float strengthGlobal = 0;
bool fingerDetected = false;
bool fingerPresent = false;
SemaphoreHandle_t dataMutex;
void clearLCDLine(uint8_t line)
{
    lcd.setCursor(0, line);
    lcd.print("                ");
    lcd.setCursor(0, line);
}

float calculatePulseStrength(uint32_t *buffer, int size)
{
    uint32_t maxVal = 0;
    uint32_t minVal = 0xFFFFFFFF;
    uint64_t sum = 0;

    for (int i = 0; i < size; i++)
    {
        if (buffer[i] > maxVal)
            maxVal = buffer[i];
        if (buffer[i] < minVal)
            minVal = buffer[i];
        sum += buffer[i];
    }

    float dc = sum / size;
    float ac = maxVal - minVal;

    if (dc == 0)
        return 0;

    return (ac / dc) * 100.0;
}

void printSerialData()
{
    Serial.print("IR=");
    Serial.print(irValueGlobal);
    Serial.print(" Strength=");
    Serial.println(strengthGlobal);

    Serial.print("SpO2: ");

    if (g_data.validSPO2)
        Serial.print(g_data.spo2);
    else
        Serial.print("---");

    Serial.print(" HR: ");

    if (g_data.validHR)
        Serial.print(g_data.heartRate);
    else
        Serial.print("---");

    Serial.print(" Temp: ");

    if (g_data.validTemp)
        Serial.print(g_data.temperature, 1);
    else
        Serial.print("--");

    Serial.println();
}

bool isDataInRange()
{
    bool spo2OK = (g_data.validSPO2 && g_data.spo2 >= SPO2_MIN && g_data.spo2 <= SPO2_MAX);
    bool hrOK = (g_data.validHR && g_data.heartRate >= HR_MIN && g_data.heartRate <= HR_MAX);
    bool tempOK = g_data.validTemp;

    return (spo2OK || hrOK || tempOK);
}

void TaskTemperature(void *pvParameters)
{
    analogReadResolution(12);
    analogSetPinAttenuation(Thermester_PIN, ADC_11db);

    const float BETA = 3950;
    const float R0 = 10000;
    const float ROOM_TEMP = 298.15;

    for (;;)
    {
        int raw = 0;

        for (int i = 0; i < 20; i++)
        {
            raw += analogRead(Thermester_PIN);
            delay(2);
        }

        raw /= 20;

        if (raw > 10 && raw < 4090)
        {
            float resistance = 10000.0 * raw / (4095.0 - raw);
            float tempK = 1.0 / ((log(resistance / R0) / BETA) + (1.0 / ROOM_TEMP));

            xSemaphoreTake(dataMutex, portMAX_DELAY);
            g_data.temperature = tempK - 273.15;
            g_data.validTemp = true;
            xSemaphoreGive(dataMutex);
        }
        else
        {
            xSemaphoreTake(dataMutex, portMAX_DELAY);
            g_data.validTemp = false;
            xSemaphoreGive(dataMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskSensor(void *pvParameters)
{
    Serial.println("Sensor Task Start");

    if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
    {
        Serial.println("MAX30102 not found");
        while (1)
            ;
    }

    particleSensor.setup(60, 4, 2, 100, 411, 4096);
    // particleSensor.setup(40, 4, 2, 100, 411, 16384);
    // particleSensor.setPulseAmplitudeRed(0x3F);
    // particleSensor.setPulseAmplitudeIR(0x3F);
      particleSensor.setPulseAmplitudeRed(0x7F);
    particleSensor.setPulseAmplitudeIR(0x7F);
    
    particleSensor.setPulseAmplitudeProximity(0);

    bool firstTime = true;
    bool collectingSpo2 = false;

    int32_t spo2History[HISTORY_SIZE] = {0};
    int32_t hrHistory[HISTORY_SIZE] = {0};
    int historyIndex = 0;

    for (;;)
    {
        particleSensor.check();

        if (!particleSensor.available())
        {
            vTaskDelay(1);
            continue;
        }

        uint32_t irValue = particleSensor.getIR();
        uint32_t redValue = particleSensor.getRed();
        particleSensor.nextSample();

        irValueGlobal = irValue;

        /* ---------- FINGER DETECTION ---------- */

        if (irValue < 15000)
        {

            fingerPresent = false;
            firstTime = true;
            collectingSpo2 = false;

            clearLCDLine(0);
            lcd.setCursor(0, 0);
            lcd.print("Health Monitor");

            clearLCDLine(1);
            lcd.setCursor(0, 1);
            lcd.print("Insert Finger");

            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }
        else
        {
            fingerPresent = true;
            collectingSpo2 = true;
        }

        /* ---------- BUFFER COLLECTION ---------- */

        if (firstTime)
        {
            for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
            {
                while (!particleSensor.available())
                    particleSensor.check();

                irBuffer[i] = particleSensor.getIR();
                redBuffer[i] = particleSensor.getRed();
                particleSensor.nextSample();
            }

            firstTime = false;
        }
        else
        {
            memmove(irBuffer, irBuffer + 25, sizeof(uint32_t) * (SAMPLE_BUFFER_SIZE - 25));
            memmove(redBuffer, redBuffer + 25, sizeof(uint32_t) * (SAMPLE_BUFFER_SIZE - 25));

            for (int i = 75; i < 100; i++)
            {
                while (!particleSensor.available())
                    particleSensor.check();

                irBuffer[i] = particleSensor.getIR();
                redBuffer[i] = particleSensor.getRed();
                particleSensor.nextSample();
            }
        }

        /* ---------- SIGNAL STRENGTH ---------- */

        strengthGlobal = calculatePulseStrength(irBuffer, SAMPLE_BUFFER_SIZE);
        bool weakSignal = (strengthGlobal < 0.02);

        /* ---------- SPO2 + HR ---------- */

        int32_t spo2, hr;
        int8_t validSpo2, validHR;

        maxim_heart_rate_and_oxygen_saturation(
            irBuffer, SAMPLE_BUFFER_SIZE, redBuffer,
            &spo2, &validSpo2,
            &hr, &validHR);

        if (validHR && validSpo2)
        {
            hrHistory[historyIndex] = hr;
            spo2History[historyIndex] = spo2;

            historyIndex = (historyIndex + 1) % HISTORY_SIZE;

            int hrSum = 0;
            int spo2Sum = 0;

            for (int i = 0; i < HISTORY_SIZE; i++)
            {
                hrSum += hrHistory[i];
                spo2Sum += spo2History[i];
            }

            xSemaphoreTake(dataMutex, portMAX_DELAY);

            g_data.heartRate = hrSum / HISTORY_SIZE;
            g_data.spo2 = spo2Sum / HISTORY_SIZE;
            g_data.validHR = true;
            g_data.validSPO2 = true;

            xSemaphoreGive(dataMutex);
        }

        /* ---------- LCD DISPLAY ---------- */

        clearLCDLine(0);
        lcd.setCursor(0, 0);

        lcd.print("SpO2:");
        if (g_data.validSPO2)
            lcd.print(g_data.spo2);
        else
            lcd.print("--");

        lcd.print("% T:");

        if (g_data.validTemp)
            lcd.print(g_data.temperature, 1);
        else
            lcd.print("--");

        lcd.print("C");

        clearLCDLine(1);
        lcd.setCursor(0, 1);

        if (weakSignal)
        {
            lcd.print("Weak Signal");
        }
        else
        {
            lcd.print("HR:");
            if (g_data.validHR)
                lcd.print(g_data.heartRate);
            else
                lcd.print("---");
            lcd.print(" bpm");
        }

        printSerialData();

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}
void TaskCloud(void *pvParameters)
{
    unsigned long lastSend = 0;

    for (;;)
    {
        if (millis() - lastSend > 1000)
        {
            xSemaphoreTake(dataMutex, portMAX_DELAY);

            int spo2 = g_data.spo2;
            int hr = g_data.heartRate;
            float temp = g_data.temperature;

            bool spo2Valid = g_data.validSPO2;
            bool hrValid = g_data.validHR;
            bool tempValid = g_data.validTemp;

            xSemaphoreGive(dataMutex);
            
            if(fingerPresent && spo2 > 0 && spo2 >0 )
            {
               saveToFirebaseRTDB(spo2, hr, temp, true, true, true); 
            }

            /* -------- Upload SpO2 -------- */
            if (spo2Valid && spo2 >= SPO2_MIN && spo2 <= SPO2_MAX)
            {
               saveToFirebaseRTDB(spo2, -1, -1, true, false, false);
                saveToFirestore(spo2, -1, -1, true, false, false);
            }

            /* -------- Upload Heart Rate -------- */
            if (hrValid && hr >= HR_MIN && hr <= HR_MAX)
            {
                //saveToFirebaseRTDB(-1, hr, -1, false, true, false);
                saveToFirestore(-1, hr, -1, false, true, false);
            }

            /* -------- Upload Temperature -------- */
            if (tempValid)
            {
              //  saveToFirebaseRTDB(-1, -1, temp, false, false, true);
                saveToFirestore(-1, -1, temp, false, false, true);
            }

            /* -------- Buzzer if any uploaded -------- */
            if (fingerPresent &&
                ((spo2Valid && spo2 >= SPO2_MIN && spo2 <= SPO2_MAX) ||
                 (hrValid && hr >= HR_MIN && hr <= HR_MAX) ||
                 tempValid))
            {
                digitalWrite(BUZZER_PIN, HIGH);
                vTaskDelay(pdMS_TO_TICKS(80));
                digitalWrite(BUZZER_PIN, LOW);
            }

            lastSend = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);

    Wire.begin(21, 22);

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Health Monitor");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    lcd.setCursor(0, 1);
    lcd.print("Connecting WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected");

    configTime(19800, 0, "pool.ntp.org");

    initFirebase(FIREBASE_API_KEY, FIREBASE_DB_URL, FIRESTORE_PROJECT);
    firestoreInit(FIREBASE_API_KEY, FIRESTORE_PROJECT);
    fetchActivePatientUID();
    lcd.setCursor(0, 1);
    lcd.print("Firebase ok");
    dataMutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(TaskTemperature, "Temp", 2000, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskSensor, "Sensor", 5000, NULL, 2, &SensorTaskHandle, 0);
    xTaskCreatePinnedToCore(TaskCloud, "Cloud", 8000, NULL, 1, &CloudTaskHandle, 1);

    lcd.setCursor(0, 1);
    lcd.print("Welcome !");
}

void fetchActivePatientUID()
{
    if (!Firebase.ready() || !fbdo)
        return;

    String path = "/device_config/ESP32_001/active_uid";
    Serial.print("Reading UID: ");
    Serial.println(path);

    if (Firebase.RTDB.getString(fbdo, path))
    {
        String newUID = fbdo->stringData();
        if (newUID.length() > 5)
        {
            activePatientUID = newUID;
            Serial.println("Active UID:");
            Serial.println(activePatientUID);
        }
        else
        {
            Serial.println("Using default UID");
            activePatientUID = DEVICE_ID;
        }
    }
    else
    {
        Serial.println("Read Failed → Default UID");
        activePatientUID = DEVICE_ID;
    }
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(200));
}