PRODUCT_ID(6812);
PRODUCT_VERSION(7);

// command stub
String command = "APP#AC:CF:23:28:C2:2C#CMD#";
// exposed cloud function
int setLight(String cmd);

// comms
TCPClient rssiClient;
byte rssiSERVER[] = {178,62,58,245};
int rssiPORT = 38899;

void setup()
{
    // opens serial over USB
     Serial.begin(9600);
    // 400Khz seems to work best with the Photon with the packaged I2C sensors
    Wire.setSpeed(CLOCK_SPEED_400KHZ);
    // register the cloud function
    Particle.function("setLight", setLight);
}

void loop() {}

int setLight(String cmd)
{
  String temp = command;
  temp += cmd + "\n";
  sendMessage(temp);
  //Serial.println(signal);
}

void sendMessage(String signal)
{
  if(rssiClient.connect(rssiSERVER, rssiPORT))
  {
    if(rssiClient.connected())
    {
      rssiClient.println(signal);
    }
    rssiClient.stop();
  }
  else
  {
    Serial.println("Cannot connect to server");
  }
}
