
// www.arduinesp.com //
// Plot DTH11 data on thingspeak.com using an ESP8266 
// August 2nd 2015
// Author: Ken Spottiswoode

// Program with Arduino IDE with ESP8266 Generic as selected Board

// Side gate reed switch is NO (no magnet near).  When gate closed, reed will be closed. A Pull-Down resistor is used.

/*-----( Import needed libraries )-----*/
#include <OneWire.h>
#include <DallasTemperature.h> 
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <switch.h>
// #include <DHT.h>
#include <SimpleTimer.h> // for Timer interrupts
SimpleTimer PostRequestTimer; // timer object

// required for Serial Streaming using <<
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

extern "C" {
#include "user_interface.h"
uint16 readvdd33(void);
}

/*-----( Declare Global variables )-----*/
// int GateSwitchPin = 5; // Connect to Digital Pin 5.  
String gstr_sensorAddress = "";
int i;
byte GateState;
String str_TempC;
float f_InternalVdd;

/*-----( Declare Constants )-----*/
#define ONE_WIRE_BUS 4 /*-(Connect to Digital Pin GPIO4 )-*/
const byte GateContactPin = 5; 
const int NetworkStatusLedPin =  12;      // GPIO12 BLUE

//Switch buttonGND = Switch(buttonGNDpin); // button to GND, use internal 20K pullup resistor
//Switch toggleSwitch = Switch(toggleSwitchpin); 
Switch GateContact = Switch(GateContactPin, INPUT, HIGH); // button to VCC, 10k pull-down resistor, no internal pull-up resistor, HIGH polarity
//Switch button10ms = Switch(Button10mspin, INPUT_PULLUP, LOW, 1); // debounceTime 1ms


 
// replace with your channel's thingspeak API key, 
String apiKey = "KQWU4ANY7SYT4LJV";
// const char* ssid = "SKYCBD3E";  Bankhead Terrace
// const char* password = "VFRJTZFT"; Bankhead Terrace

const char* ssid = "SKY5B869"; // Lilybank Crescent
const char* password = "TFPTXTVU"; //Lilybank Crescent
const char* server = "api.thingspeak.com"; // "184.106.153.149" or api.thingspeak.com

static char str_vdd[15]; // for Vdd float to String conversion

// #define DHTPIN 2 // what pin we're connected t
//DHT dht(DHTPIN, DHT11,15);

/*-----( Declare objects )-----*/
OneWire ourWire(ONE_WIRE_BUS);  /* Set up a oneWire instance to communicate with any OneWire device*/

/*-----( Initialise objects )-----*/
DallasTemperature sensors(&ourWire); /* Tell Dallas Temperature Library to use oneWire Library */

WiFiClient client; // ESP8266 Wifi start

 
void setup() 
{
	// set the LED  pin as output:
	pinMode(NetworkStatusLedPin, OUTPUT);
  
	PostRequestTimer.setInterval(15000,PostHTTPRequest); // Thingspeak requires 15 secs between transmissions - not sure about receives

	Serial.begin(115200);
	delay(10);
	// dht.begin();

	WiFi.begin(ssid, password);

	Serial.print("\n\rConnecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
	}
	Serial.println("\n\rWiFi connected");

	/* *********************************** Start up the DallasTemperature library **** */
	sensors.begin();

	// Display Sensor HEX address only during Setup
	discoverOneWireDevices(gstr_sensorAddress); // Find and return Hex address as string pointer
	
	Serial << "\n\r\n\rFound \'1-Wire\' device having HEX address:\n\r" << gstr_sensorAddress << "\n\r";
}
 
 
void loop() 
{
 	/*-----( Declare Variables )-----*/
	
	// float h = dht.readHumidity();
	// float t = dht.readTemperature();
	// if (isnan(h) || isnan(t)) 
	//	{
		// Serial.println("Failed to read from DHT sensor!");
		// return; 
	//	}
	
	PostRequestTimer.run(); // run interval Timer
	
	GateState = GetGateState();			// Check status of gate switch contact
	f_InternalVdd = readVdd();			// read ESP8266 ADC value
	str_TempC = readTempSensor1();		// get Temperature Sensor 1 temperature in Deg Celsius
	
	Serial << "Free Heap Size: " << system_get_free_heap_size() << " MB\n\r"; // report free Heap Memory
	delay(2000);  
}
/* *******************************  END OF MAIN LOOP  ************************************************** */



/* *********************************  HTTP Request  ************************************************** */
void  PostHTTPRequest()
{
	Serial << "attempting connection to " << server << "\n\r\n\r";
	WiFiClient client;
	const int httpPort = 80;
	// connect and post to ThingsSpeak Channel
	if (client.connect(server, httpPort)) 
	  { 
		String postStr = apiKey;
		postStr +="&field1=";
		postStr += GateState;
		postStr +="&field2=";
		postStr += f_InternalVdd;
		postStr +="&field3=";
		postStr += str_TempC;
		postStr += "\r\n\r\n";
	 
		client.print("POST /update HTTP/1.1\n"); 
		client.print("Host: api.thingspeak.com\n"); 
		client.print("Connection: close\n"); 
		client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n"); 
		client.print("Content-Type: application/x-www-form-urlencoded\n"); 
		client.print("Content-Length: "); 
		client.print(postStr.length()); 
		client.print("\n\n"); 
		client.print(postStr);
			   
		Serial << "Data Posted to Thingspeak: \n\r" << postStr << "\n\r\n\r";  
		delay(10);
		blinkLED(NetworkStatusLedPin);	// show visual connection		
	}
	client.stop();
}


/* ************************************ Get Gate Status ************************************* */
boolean GetGateState(void)
{
	boolean ContactState;
	
	
	GateContact.poll();		// Check de-bounced status of gate switch contact
	if (GateContact.on())
		ContactState = 1;
		else
			ContactState = 0;
			
	if (ContactState)
		Serial << "Gate is CLOSED.\n\r";
	else
		Serial << "Gate is OPEN.\n\r";
		
	return ContactState;
}


/* ************************************  Blink an LED ************************* */
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


/* ************************************ read 1-wire DS18B20 temperature sensor No 1 ********* */
/* String readTempSensor1(void)
{
	String str_TempC;
	
	Serial.print("\n\rRequesting sensor temperature...");
	sensors.requestTemperatures(); // Send the command to get temperatures
//	Serial << "DONE" << "Device 1 (index 0) = ";
	str_TempC = sensors.getTempCByIndex(0);
	Serial << str_TempC << " Degrees C\n\r";
	return str_TempC;
} */
String readTempSensor1(void)  // modified this function as I had been reading the sensors.getTempC into a string, when it should be a Float
{
	char str_TempC[6];
	float TempC;
	
	Serial.print("\n\rRequesting sensor temperature...");
	sensors.requestTemperatures(); // Send the command to get temperatures
//	Serial << "DONE" << "Device 1 (index 0) = ";
	TempC = sensors.getTempCByIndex(0);
	Serial << TempC << " Degrees C\n\r";
	dtostrf(TempC,5, 2, str_TempC);
//	Serial << "Temperature: " << str_TempC " Deg C";
	Serial.println(str_TempC);
	return str_TempC;
}
	
	

/* ************************************ Return  ADC measured Internal voltage ****** */
float readVdd(void)
{
	float vdd = readvdd33() / 1000.0; // read internal ADC Voltage
	dtostrf(vdd,7, 2, str_vdd);
	Serial << "Vdd: " << str_vdd << "\n\r";
	return vdd;
}



/* ******************************************** Declare User-written Functions ****** */
void discoverOneWireDevices(String &sensorAddr) 
{
  byte i;
  byte addr[8];

  Serial.print("Looking for 1-Wire devices...\n\r");// "\n\r" is NewLine 
  while(ourWire.search(addr)) 
  {
	//----------------------------------------------
	//Get the Sensor-Address in HEX (8 bytes long)
	//----------------------------------------------
	for(i = 0; i < 8; i++) 
	{
		//Add a leading zero if Hex is less than or equal to 15
		if(addr[i] <= 0x0F)
		{
			sensorAddr = sensorAddr + "0";
		}
		sensorAddr = sensorAddr + String(addr[i], HEX);
		if(i < 7)
		{
			sensorAddr = sensorAddr + ':';
		}
	}
	sensorAddr.toUpperCase();
  }
  ourWire.reset_search();
  return;
}
