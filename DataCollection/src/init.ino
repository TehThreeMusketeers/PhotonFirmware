// compiling for products
PRODUCT_ID(6812);
PRODUCT_VERSION(5);

#include "Si1132.h"
#include "Si70xx.h"

const int SOUND = A0;
double SOUNDV = 0; //// Volts Peak-to-Peak Level/Amplitude
// exposed on Particle Cloud
double soundValue = 0;

// Air Temperature
bool Si7020OK = false;
// exposed on Particle Cloud
double Si7020Temp = 0; //// Celsius

// Ambient light
bool Si1132OK = false;
// exposed on Particle Cloud
double Si1132Vis = 0; //// Lux
Si1132 si1132 = Si1132();

void setup()
{
  Serial.begin(9600);
  // 400Khz seems to work best with the Photon with the packaged I2C sensors
  Wire.setSpeed(CLOCK_SPEED_400KHZ);
  // register the cloud variables
  Particle.variable("sound", soundValue);
  Particle.variable("temp", Si7020Temp);
  Particle.variable("light", Si1132Vis);
  //soundworks
  pinMode(SOUND, INPUT);
}

void loop()
{
  readSi1132Sensor();
  readWeatherSi7020();
  soundValue = readSoundLevel();
  delay(1000);
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

///reads UV, visible and InfraRed light level
void readSi1132Sensor()
{
    si1132.begin(); //// initialises Si1132
    Si1132Vis = si1132.readVisible();
}

void readWeatherSi7020()
{
    Si70xx si7020;
    Si7020OK = si7020.begin(); //// initialises Si7020

    if (Si7020OK)
    {
        Si7020Temp = si7020.readTemperature();
    }
}
