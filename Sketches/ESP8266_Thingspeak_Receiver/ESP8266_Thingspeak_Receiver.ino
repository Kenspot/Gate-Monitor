/*
 *	Reads a Thingspeak Channel 'Field' last value.
 *	Uses bits of code from : zoomkat 12-16-11 simple client test to find what lines of the request
 *	are read back in variable 'x' and substring() function to parse the 'last value'.
 */
 
#include <ESP8266WiFi.h>
#include <SimpleTimer.h>// for Timer interrupts
SimpleTimer GetRequestTimer; // timer object

// required for Serial Streaming using <<
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

//data from myIP server captured in readString
String readString, readString1, data;
int x=0, lastValue, lastGateState;
char lf=10; // Line Feed character

int NetworkStatusLEDState = LOW;
int GateClosedLEDState = LOW;
int GateOpenLEDState = LOW;


// constants won't change. 

// set LED pin numbers:
const int NetworkStatusLedPin =  12;      // GPIO12 BLUE
const int GateClosedLedPin =  13;	// GPIO13 GREEN
const int GateOpenLedPin =  14;	// GPIO14 WHITE  ()RED)

// const char* ssid = "SKYCBD3E";  Bankhead Terrace
// const char* password = "VFRJTZFT"; Bankhead Terrace

const char* ssid = "SKY5B869"; // Lilybank Crescent
const char* password = "TFPTXTVU"; //Lilybank Crescent
const char* host = "api.thingspeak.com"; // "184.106.153.149" or api.thingspeak.com


void setup() {

// set the LEDS  pins as outputs:
  pinMode(NetworkStatusLedPin, OUTPUT);
  pinMode(GateClosedLedPin, OUTPUT);
  pinMode(GateOpenLedPin, OUTPUT);
  
  GetRequestTimer.setInterval(15000,GetHTTPRequest); // Thingspeak requires 15 secs between transmissions - not sure about receives

  Serial.begin(115200);
  delay(10);
 
  // We start by connecting to a WiFi network
  Serial << "Connecting to WiFi Network with name: " << ssid << "\n\r";
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial << "\n\r" << "WiFi now connected\n\r"  << "IP address: " << WiFi.localIP() << "\n\r\n\r";
}
 

/* ********************************  MAIN LOOP   **************************************** */
void loop() 
{

	GetRequestTimer.run(); // run interval Timer

    // if the LED is off turn it on and vice-versa:
    if (lastGateState) // 	1/True is 'closed'
	{
		GateClosedLEDState = HIGH;
		GateOpenLEDState = LOW;
	}
    else
	{
		GateClosedLEDState = LOW;
		GateOpenLEDState = HIGH;
	}

    // set the LED with the ledState of the variable:
    digitalWrite(GateClosedLedPin, GateClosedLEDState);
	digitalWrite(GateOpenLedPin, GateOpenLEDState); 
}

void blinkLED(byte ledPIN)
{
	//  consumes 300 ms.
	for(int i = 5; i>0; i--){
	digitalWrite(ledPIN, HIGH);
	delay(30);
	digitalWrite(ledPIN, LOW);
	delay(30);
  }
}
  
void  GetHTTPRequest()
{
	Serial << "attempting connecting to " << host << "\n\r\n\r";
	WiFiClient client;
	const int httpPort = 80;
	if (client.connect(host, httpPort)) {
		String url = "/channels/42015/fields/1/last";
		Serial << "Requesting URL: " << url << "\n\r";
		
		String Request;
		Request = "GET ";
		Request += url;
		Request += " HTTP/1.0\r\n";
		Request += "Host: api.thingspeak.com \r\n";
		Request += "Connection: close\r\n\r\n";
		client.print(Request);
	 
		delay(10);
		blinkLED(NetworkStatusLedPin);	// show visual connection
	}
	else
		{
		Serial.println("connection failed");
		return;
	}
	
	
	// Read all the lines of the reply from server and print them to Serial
	while (client.available()) 
	{
		char c = client.read();
		Serial.println("Lilybank WIFI CONNECTED");
		Serial.print(c);
		if (c==lf) x=(x+1); // Count Line Feed characters
		if (x==17) readString += c; // 'x' would be required line Number to find
	}
	
	Serial.flush();
	Serial.print("\n\r\n\r closing connection");
	Serial << "\n\r\n\r" << "Current data row showing text :"  << readString << "\n\r";
	
	readString1 = (readString.substring(0,2)); // don't know why but I have to add 1 to the Len value.
	lastGateState =(readString1.toInt());  // convert string to Integer

	Serial << "\n\r" << "Found following string: " << readString1 << "\n\r" ;
	Serial << "\n\r" << "Integer value: " << lastGateState << "\n\r" ;
	
	// clear variables for new input
	readString1 = ""; 
	readString = ""; 
	x=0;
	Serial << "done\n\r\n\r\n\r";
}






/*
void sendGET() //client function to send/receive GET request data.
{
  if (client.connect(serverName, 80)) {  //starts client connection, checks for connection
    Serial.println("connected");
    client.println("GET /channels/21652/fields/3/last HTTP/1.1"); //download text
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");  //close 1.1 persistent connection 
    client.println(); //end of get request
  }
  else {
    Serial.println("connection failed"); //error message if no client connect
    Serial.println();
  }

  while(client.connected() && !client.available()) delay(1); //waits for data
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer
    readString += c; //places captured byte in readString
  }

  //Serial.println();
  client.stop(); //stop client
  Serial.println("client disconnected.");
  Serial.println("Data from server captured in readString:");
  Serial.println();
  Serial.print(readString); //prints readString to serial monitor
  Serial.println(); 
  Serial.println();
  Serial.println("End of readString");
  Serial.println("==================");
  Serial.println();
  int loc = readString.indexOf("4\r\n");
  data = readString.substring(loc+3, loc+9);
  Serial.print("data is: ");
  Serial.println(data);
  Serial.println();
  readString=""; //clear readString variable
  data="";

}
*/
