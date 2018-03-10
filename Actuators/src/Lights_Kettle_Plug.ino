
#include <HttpClient.h>
#include <Particle.h>
PRODUCT_ID(6812);
PRODUCT_VERSION(10);


// HTTP communication
HttpClient http;
http_request_t request;
http_response_t response;
http_header_t headers[] = {
  {"Accept", "text/plain"},
  {NULL,NULL}
};

// command stubs
String lightCommand = "APP#AC:CF:23:28:C2:2C#CMD#";
String kettleCommand = "set sys output ";
String plugCommand = "/cgi-bin/relay.cgi?";
// exposed cloud functions
int setLight(String cmd);
int setKettle(String cmd);
int setPlug(String cmd);

// comms
TCPClient rssiClient;
byte lightAddress[] = {178,62,58,245};
int lightPORT = 38899;
byte kettleAddress[] = {192,168,0,104};
int kettlePort = 2000;

String tst;

void setup()
{
    // opens serial over USB
     Serial.begin(9600);
    // 400Khz seems to work best with the Photon with the packaged I2C sensors
    Wire.setSpeed(CLOCK_SPEED_400KHZ);
    // register the cloud function
    Particle.function("setLight", setLight);
    Particle.function("setKettle", setKettle);
    Particle.function("setPlug", setPlug);
}

void loop() {}

int setLight(String cmd)
{
  String temp = lightCommand;
  temp += cmd + "\n";
  sendMessage(temp, lightAddress, lightPORT);
}

int setKettle(String cmd)
{
  String temp = kettleCommand;
  temp += cmd + "\n";
  sendMessage(temp, kettleAddress, kettlePort);
}

int setPlug(String cmd)
{
  String temp = plugCommand + cmd;
  callPlug(temp);
}

void callPlug(String cmd)
{
  request.hostname = "192.168.0.107";
  request.port = 80;
  request.path = cmd;
  http.get(request, response, headers);
  //printResponse(response);
}

// print the data returned from the server for development and testing purposes
void printResponse(http_response_t &response)
{
  Serial.println("HTTP Response: ");
  Serial.println(response.status);
  Serial.println(response.body);
}

void sendMessage(String signal, byte server[], int port)
{
  if(rssiClient.connect(server, port))
  {
    if(rssiClient.connected())
    {
      rssiClient.println(signal);
    }
  }
  else
  {
    Serial.println("Cannot connect to server");
  }
  rssiClient.flush();
  rssiClient.stop();
}
