// Copyright Benoit Blanchon 2014
// MIT License
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson
#include <ArduinoJson-v5.13.0.h>
#include <HttpClient.h>
#include <Particle.h>
#include "Si1132.h"

// compiling for products
PRODUCT_ID(6812);
PRODUCT_VERSION(3)

// light reafing variables
Si1132 light;
float lightThreshold;
float currentLight;
String lightOperator = "";
float userLightThreshold = 0;

// sound reading variables
const int SOUND = A0;
double SOUNDV = 0; //// Volts Peak-to-Peak Level/Amplitude
double soundValue;
double soundThreshold = 0;
String soundOperator = "";
double userSoundThreshold = 0;

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
// received with each trigger definition
String deviceId = "";
String triggerId = "";
// generated based on trigger data
String eventData = "";
String extraEventData = "";

// variabeles for holding trigger data - initialised with default values
String triggerState = "";
bool triggerMovement = false;
bool triggerSound = false;
bool triggerLight = false;
String triggerType = "";

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
  // initialise light sensor
  light.begin();
  delay(3000);
  // initialise Sound Sensor and do initial calibration
  pinMode(SOUND, INPUT);
  soundThreshold = readSoundLevel();
  soundThreshold = soundThreshold + 0.5;
  // LED
  pinMode(LED, OUTPUT);
  // PIR Sensor
  pinMode(MOVEMENT, INPUT);
}

void loop()
{
  delay(1000);
  currentLight = light.readVisible();
  if(currentState.equals("ARMED"))
  {
    if(triggerMovement)
    {
      if(monitorMovement())
      {
        Serial.println("Movement detected while in state armed - event raised.");
        Particle.publish("eventname", eventData);
        if(triggerState.equals("ARMED"))
        {
          // start testing the trigger conditions
          // TODO two different triggers sound and light
          if(triggerSound && triggerLight)
          {
            Serial.println("I monitoring sound and light.");
            //when sound and light threshold is greater or equal
            if((soundOperator.equals(">") || soundOperator.equals(">=")) && (lightOperator.equals(">") || lightOperator.equals(">=")))
            {
              if((readSoundLevel() >= userSoundThreshold) && (currentLight >= userLightThreshold))
              {
                Serial.println("current sound is greater and current light is greater");
                tellServer();
              }
            }
            //when sound and light threshold is lower or equal
            else if ((soundOperator.equals("<") || soundOperator.equals("<=")) && (lightOperator.equals("<") || lightOperator.equals("<=")))
            {
              if((readSoundLevel() <= userSoundThreshold) && (currentLight <= userLightThreshold))
              {
                Serial.println("current sound is smaller and current light is smaller");
                tellServer();
              }
            }
            //when sound is greater and light threshold is lower
            else if ((soundOperator.equals(">") || soundOperator.equals(">=")) && (lightOperator.equals("<") || lightOperator.equals("<=")))
            {
              if((readSoundLevel() >= userSoundThreshold) && (currentLight <= userLightThreshold))
              {
                Serial.println("current sound is greater and current light is smaller");
                tellServer();
              }
            }
            //when sound is lower and light threshold is greater
            else if ((soundOperator.equals("<") || soundOperator.equals("<=")) && (lightOperator.equals(">") || lightOperator.equals(">=")))
            {
              if((readSoundLevel() <= userSoundThreshold) && (currentLight >= userLightThreshold))
              {
                Serial.println("current sound is smaller and current light is greater");
                tellServer();
              }
            }
          }
          // sound only
          else if (triggerSound)
          {
            //check sound
            if(soundOperator.equals(">") || soundOperator.equals(">="))
            {
              if(readSoundLevel() >= userSoundThreshold)
              {
                Serial.println("current sound is greater");
                tellServer();
              }
            }
            else if (soundOperator.equals("<") || soundOperator.equals("<="))
            {
              if(readSoundLevel() <= userSoundThreshold)
              {
                Serial.println("current sound is lower");
                tellServer();
              }
            }
          }
          // light only
          else if (triggerLight)
          {
            //check light
            if(lightOperator.equals(">") || lightOperator.equals(">="))
            {
              if(currentLight >= userLightThreshold)
              {
                Serial.println("current light is greater");
                tellServer();
              }
            }
            else if (lightOperator.equals("<") || lightOperator.equals("<="))
            {
              if(currentLight <= userLightThreshold)
              {
                Serial.println("current light is lower");
                tellServer();
              }
            }
          }
        }
      }

    }
  }
  //delay(1000);
}

// tell the server if the condition set in the trigger has been met
void tellServer()
{
  extraEventData = "{\"conditionMet\": \"true\"};"
  Particle.publish("conditionMet", extraEventData);
  extraEventData = "";
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

  // testing data
  //request.hostname = "52324f10-a550-4316-bb9d-8338ed55f4b8.mock.pstmn.io";
  //request.path = "/devices/trigger/";
  //request.body = "{\"someKey\":\"someValue\"}";
  //request.port = 80;

  http.get(request, response, headers);
  printResponse(response);

  if(!((response.status == 200) || (response.status == 201)))
  {
    Serial.print("Response status: ");
    Serial.println(response.status);
    return -1;
  }
  //test print
  //printResponse(response);
  StaticJsonBuffer<300> jsonBuffer;
  String j = response.body;
  // https://github.com/bblanchon/ArduinoJson/issues/179
  JsonObject& root = jsonBuffer.parseObject(const_cast<char*>(j.c_str()));
  // check if json string was parsed successfully - on failure return -2
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return -2;
  }

  // e.g. {"id": 1, "valuetype": "light", "state": "ARMED", "operator": ">=", "device": 9, "value": "30"}
  int id = root["id"];
  const char* jTriggerType = root["valuetype"];
  triggerType = (String)jTriggerType;
  const char* jOperator = root["operator"];
  String oprtr = (String)jOperator;
  int dev = root["device"];
  const char* jVal = root["value"];
  String val = (String)jVal;
  // find what type of sensor needs monitoring
  if(triggerType.equals("light"))
  {
    triggerLight = true;
    lightOperator = oprtr;
    userLightThreshold = atof(jVal);
    Serial.println("trigger of type light. user treshold: ");
    Serial.println(String(userLightThreshold));
  }
  else if (triggerType.equals("sound"))
  {
    triggerSound = true;
    soundOperator = oprtr;
    userSoundThreshold = atof(jVal);
  }

  const char* jState = root["state"];
  triggerState = (String)jState;
  deviceId = String(id);
  triggerId = String(dev);

  eventData = "{\"deviceID\":" + deviceId + ", \"trigger\":" + triggerId + "}";

  // test print
  Serial.println("----------------------------------------");
  Serial.println(triggerType);
  Serial.println(triggerState);
  Serial.println(oprtr);
  Serial.println(val);
  Serial.println(eventData);
  Serial.println("----------------------------------------");
  Serial.println();

  return 1;
}


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// from here below all set and working

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

/* exposed function to be called by django server
   the parameter represents the state to put the photon in
*/
int setState(String state)
{
  currentState = state;
  // set a fresh sound threshold to be monitored in the new state when system is armed
  if(currentState.equals("ARMED"))
  {
    // when armed, monitor movement ...
    triggerMovement = true;
    // and set the sound treshold
    soundThreshold = readSoundLevel();
    soundThreshold = soundThreshold + 0.5;
  }
  else
  {
    triggerMovement = false;
    triggerSound = false;
    triggerLight = false;
    userLightThreshold = 0;
    userSoundThreshold = 0;
    triggerState = "";
    deviceId = "";
    triggerId = "";
    eventData = "";
    soundOperator = "";
    lightOperator = "";
    extraEventData = "";
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
    Serial.println("Alarm triggered by noise.");
    return true;
  }
  return false;
}

bool monitorMovement()
{
  // the environment is monitored and a boolean value is returned depending on
  // whether movement has been detected
  if(digitalRead(MOVEMENT) == HIGH)
  {
    Serial.println("Movement detected.");
    digitalWrite(LED, HIGH);
    return true;
  }
  else
  {
    digitalWrite(LED, LOW);
    return false;
  }
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
