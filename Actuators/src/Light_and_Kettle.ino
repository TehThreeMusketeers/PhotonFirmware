PRODUCT_ID(6812);
PRODUCT_VERSION(9);

// command stubs
String lightCommand = "APP#AC:CF:23:28:C2:2C#CMD#";
String kettleCommand = "set sys output ";
// exposed cloud functions
int setLight(String cmd);
int setKettle(String cmd);

// comms
TCPClient rssiClient;
byte lightAddress[] = {178,62,58,245};
int lightPORT = 38899;
byte kettleAddress[] = {192,168,0,104};
int kettlePort = 2000;

void setup()
{
    // opens serial over USB
     Serial.begin(9600);
    // 400Khz seems to work best with the Photon with the packaged I2C sensors
    Wire.setSpeed(CLOCK_SPEED_400KHZ);
    // register the cloud function
    Particle.function("setLight", setLight);
    Particle.function("setKettle", setKettle);
}

void loop() {}

int setLight(String cmd)
{
  String temp = lightCommand;
  temp += cmd + "\n";
  sendMessage(temp, lightAddress, lightPORT);
  Serial.println(temp);
}

int setKettle(String cmd)
{
  String temp = kettleCommand;
  temp += cmd + "\n";
  Serial.println(temp);
  sendMessage(temp, kettleAddress, kettlePort);
  return 1;
}

void sendMessage(String signal, byte server[], int port)
{
  if(rssiClient.connect(server, port))
  {
    if(rssiClient.connected())
    {
      rssiClient.println(signal);
    }
    rssiClient.flush();
    rssiClient.stop();
  }
  else
  {
    Serial.println("Cannot connect to server");
  }
}
