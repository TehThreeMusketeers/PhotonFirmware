// Copyright Benoit Blanchon 2014
// MIT License
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson
#include <ArduinoJson-v5.13.0.h>
#include <HttpClient.h>
#include <Particle.h>

// compiling for products
PRODUCT_ID(6812);
PRODUCT_VERSION(3)

// sound reading variables
const int SOUND = A0;
double SOUNDV = 0; //// Volts Peak-to-Peak Level/Amplitude
double soundValue;
double soundThreshold = 0;

//movement reading variables
int sensorValue = 0;
int sensorState = LOW;
int MOVEMENT = D6;

// onboard led
int LED = D7;

// exposed cloud functions and variables
int setTrigger(String url);
int setState(String state);
String currentState = "default";


// HTTP communication
HttpClient http;
http_request_t request;
http_response_t response;
http_header_t headers[] = {
  {"Content-Type", "application/json"},
  {"Accept", "application/json"},
  {NULL,NULL}
};

void setup()
{
  Serial.begin(9600);
  // 400Khz seems to work best with the Photon with the packaged I2C sensors
  Wire.setSpeed(CLOCK_SPEED_400KHZ);
  // register the cloud function
  Particle.function("setTrigger", setTrigger);
  Particle.function("setState", setState);
  Particle.variable("currentState", currentState);
  delay(3000);
  // Sound Sensor
  pinMode(SOUND, INPUT);

  // removed due to PIR sensor being faulty
  // LED
  //pinMode(LED, OUTPUT);
  // PIR Sensor
  //pinMode(MOVEMENT, INPUT);
}

void loop()
{
  // PIR sensor is faulty so cannot implement yet - waiting on a new one
  //sensorValue = digitalRead(MOVEMENT);
  if(triggerState.equals(currentState))
  {
    Serial.println("Monitoring...");
    if(triggerMovement && triggerSound)
    {
      Serial.println("movement and sound");
      if(monitorMovement() || monitorSound())
      {
        Serial.println("Sound and movement monitor: detected!");
      }
    }
    else if(triggerSound)
    {
      Serial.println("Sound...");
      if(monitorSound())
      {
        Serial.println("Sound only monitor: sound detected!");
        String toPublish = "alarm";
        Particle.publish("raiseAlarm", toPublish);
      }
    }
    else if(triggerMovement)
    {
      Serial.println("movement");
      if(monitorMovement())
      {
         Serial.println("Movement only monitor: detected!");
      }
    }
  }
  delay(300);
}

/* exposed function to be called by django server
   the parameter represents the state to put the photon in
*/
int setState(String state)
{
  currentState = state;
  // set a fresh sound threshold to be monitored in the new state when system is armed
  if(currentState.equals("ARMED"))
  {
    soundThreshold = readSoundLevel();
    soundThreshold = soundThreshold + 0.5;
  }
  Serial.print("State set to: ");
  Serial.println(currentState);
  return 1;
}

// the environment is monitored and a boolean value is returned depending on
// whether the sound threshold has beed breached
bool monitorSound()
{
  soundValue = readSoundLevel();
  if(soundValue > soundThreshold)
  {
    Serial.println("Alarm triggered.");
    return true;
  }
  return false;
}

bool monitorMovement()
{
  //PIR sensor not working correctly - :(
  // the environment is monitored and a boolean value is returned depending on
  // whether movement has been detected
  //movement detected - return true
  //return true;
  // no movement - return false
  return false;
}

/* exposed function to be called by django server
   the parameter represents the location where I can find the rules for the trigger
   set by the client app in JSON format
   the return will be -1 for failure and any positive integer represents the photon trigger id
   which is associated with a number of actions associated with the said trigger
*/
int setTrigger(String url)
{
  Serial.print(url);
  // make a GET request at the location passed as parameter which would return a json string
  return sendGetRequest(url);
}

// send a get request using the url passed as parameter
int sendGetRequest(String url)
{
  // split the url into required parts for the request object
  char hostNameToken = ':';
  char hostUrlToken = '/';
  for(int i = 0; i < url.length(); i++)
  {
    if(url.charAt(i) == hostNameToken)
    {
      request.hostname = url.substring(0, i);
      //Serial.println(request.hostname);
      for(int j = i+1; j < url.length(); j++)
      {
        if(url.charAt(j) == hostUrlToken)
        {
          request.port = url.substring(i+1, j).toInt();
          request.path = url.substring(j);
          //Serial.println(request.port);
          //Serial.println(request.path);
          break;
        }
      }
    }
  }

  http.get(request, response, headers);
  printResponse(response);

  if(!((response.status == 200) || (response.status == 201)))
  {
    Serial.print("Response status: ");
    Serial.println(response.status);
    return -1;
  }

  StaticJsonBuffer<300> jsonBuffer;
  String j = response.body;
  JsonObject& root = jsonBuffer.parseObject(const_cast<char*>(j.c_str()));

  // check if json string was parsed successfully - on failure return -2
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return -2;
  }

  // extract values from JsonObject e.g. {"state":"ARMED","movement":"true","sound":"true"}
  const char* jState = root["state"];
  triggerState = (String)jState;
  triggerMovement = (bool)root["movement"];
  triggerSound = (bool)root["sound"];

  return 1;
}

// print the data returned from the server for development and testing purposes
void printResponse(http_response_t &response)
{
  Serial.println("HTTP Response: ");
  Serial.println(response.status);
  Serial.println(response.body);
}

// take a sound reading
float readSoundLevel()
{
    unsigned int sampleWindow = 50; // Sample window width in milliseconds (50 milliseconds = 20Hz)
    unsigned long endWindow = millis() + sampleWindow;  // End of sample window

    unsigned int signalSample = 0;
    unsigned int signalMin = 4095; // Minimum is the lowest signal below which we assume silence
    unsigned int signalMax = 0; // Maximum signal starts out the same as the Minimum signal

    // collect data for milliseconds equal to sampleWindow
    while (millis() < endWindow)
    {
        signalSample = analogRead(SOUND);
        if (signalSample > signalMax)
        {
            signalMax = signalSample;  // save just the max levels
        }
        else if (signalSample < signalMin)
        {
            signalMin = signalSample;  // save just the min levels
        }
    }
    //SOUNDV = signalMax - signalMin;  // max - min = peak-peak amplitude
    SOUNDV = mapFloat((signalMax - signalMin), 0, 4095, 0, 3.3);
    return SOUNDV;
}

// map the sound reading value
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}