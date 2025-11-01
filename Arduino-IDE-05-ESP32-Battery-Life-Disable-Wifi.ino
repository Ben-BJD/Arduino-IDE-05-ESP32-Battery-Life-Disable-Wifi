#include <Arduino.h>
#include <WiFi.h>

#define LED_BUILTIN 8// Pin number for built-in LED. If not available use an external LED

// Define your WiFi credentials
#define WIFI_SSID "<WIFI_SSID_NAME>"
#define WIFI_PASSWORD "<WIFI_PASSWORD>"

//variable to measure time taken by time intensive task
long previousMillis = 0;

//interval to wait before next wifi reconnection attempt (if connection is lost)
unsigned long wifiReconnectIntervalMs = 30000;
//millis based time reference - used for wifi reconnection
unsigned long lastTimeRecorded = 0;

//helper functions to disable wifi
void disableWifi()
{
  WiFi.disconnect();
  // Set WiFi mode to off to save power
  WiFi.mode(WIFI_OFF);
}

//helper function to enable wifi
void enableWifi()
{
  //we need to set WiFi mode to STA explicitly after turning it off
  WiFi.mode(WIFI_STA);

  // Set auto reconnect WiFi or network connection
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print(".");
      delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

//A dummy time intensive task that returns the time taken in milliseconds
long timeIntensiveTask()
{
    previousMillis = millis();
    volatile uint32_t sum = 0;
    for (uint32_t i = 0; i < 60000000; i++) {
    sum += i;
    }

    return millis() - previousMillis;
}

//helper function to blink the built-in LED
void blink(int numBlinks,int blinkDelay)
{
    for(int i=0;i<numBlinks;i++)
    {
         // Blink the LED to indicate that the ESP32 is awake
        digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
        delay(blinkDelay);                      // wait
        digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
        delay(blinkDelay);                      // wait
    }
}

//helper function to perform time intensive task, print time taken and blink LED
void wasteTime(char* message)
{
  Serial.println(message);
  long time = timeIntensiveTask();
  float timeSecs = (float)time / 1000.0;

  Serial.print("Time intensive task took: ");
  Serial.print(timeSecs);
  Serial.println(" seconds");
  //blink LED twice to indicate completion of time intensive task
  blink(2,300);
}

void setup() 
{
  Serial.begin(9600);
  //Initialize built-in LED pin
  pinMode(LED_BUILTIN, OUTPUT);

  //Initial blink to indicate startup
  blink(3,300);

  //Initial time intensive task without WiFi
  wasteTime("No Wifi - proceeding with time intensive task");

  //Enable WiFi and connect to AP
  enableWifi();
}

void loop() 
{
  //Wifi reconnection handling
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every 30 seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - lastTimeRecorded >=wifiReconnectIntervalMs)) 
  {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    lastTimeRecorded = currentMillis;
  }

  if(WiFi.status() == WL_CONNECTED)
  {
    //Connected to WiFi - proceed with time intensive task
    wasteTime("WiFi Connected - proceeding with time intensive task");
    //now turn off WiFi to save power and waste more time
    disableWifi();
    wasteTime("WiFi Disconnected - proceeding with time intensive task");
    //turn wifi back on for next iteration
    enableWifi();
  }

}
