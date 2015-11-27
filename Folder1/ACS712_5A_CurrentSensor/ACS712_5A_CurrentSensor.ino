
void setup() 
{

Serial.begin(9600);
}


// *****************************************************************************************************
void loop() 
{
	int steps = 1024;
	double stepsHalved = 512;
	
	float SensorSensitivity  = 185; // Sensor sensitivity: 185mV/A
	float Vcc;
	float Vzero;
	
	float CalculatedCurrent = 0;
	float ValueRead = 0;
	float VoltageMeasured = 0;
	
	float averageValueRead = 0;
	float averageVoltageMeasured = 0;
	float averageCalculatedCurrent = 0;
	
	float Offset = -0.012;
	
	int Samples = 500;
	
	float average = 0;
		for(int i = 0; i < 100; i++) 
		{
			average+= (0.0264 * analogRead(A0) -13.51) / 1000;
			delay(10);
		}
		average+= Offset;
		Serial.println(average, DEC); 
  
/*	
	Vcc = readVcc();	//assuming that Vcc does not change significantly during sampling
//	Vzero = Vcc; 
		
	for(int i = 0; i < Samples; i++) 
	{	
		ValueRead = analogRead(A0);
		averageValueRead += ValueRead;
		delay(10); // allow ADC to settle

//		averageVoltageMeasured += ((Vcc/steps) * (ValueRead-stepsHalved) * 1000);
//		averageCalculatedCurrent += (((Vcc/steps) * (ValueRead-stepsHalved) * 1000)/SensorSensitivity * 1000);
	}
	averageValueRead = (averageValueRead/Samples) + Offset;
	
//	averageValueRead -= offset; // removes error at zero input
//	VoltageMeasured = ((Vcc/steps) * (averageValueRead-stepsHalved-offset));

	VoltageMeasured = ((Vcc/steps/2) * (averageValueRead)/1000);
	CalculatedCurrent = (VoltageMeasured / SensorSensitivity);
	
	Serial.print("Average Value Read: ");
	Serial.println(averageValueRead);
	
//	Serial.print("Average Calculated Voltage: ");
//	Serial.print(averageVoltageMeasured/Samples);
	Serial.print("Calculated Voltage: ");
	Serial.print(VoltageMeasured);
	Serial.println("mV"); 
	
//	Serial.print("Average Calculated Current: ");
//	Serial.print(averageCalculatedCurrent/Samples);
	Serial.print("Calculated Current: ");
	Serial.print(CalculatedCurrent);
	Serial.println("mA"); 
	
	Serial.println(); 
	Serial.println(); 
	*/
	delay(2000);
}



// *****************************************************************************************************
//  more accurate way to read actual Vcc - 
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}

