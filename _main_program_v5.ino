//Autor Norbert Krecmer
//verzia programu c. 5 
//Info: riadenie a meranie systemu berzpecnosti, komunikacie cez gsm, 
//meranie teploty, vlhkosti, meranie a riadenie nabyjania akumulacnych baterii, 
//ovladanie zavlazovania.
#include "Adafruit_FONA.h"
#define FONA_RST 2
HardwareSerial *fonaSerial = &Serial1;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
//Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);
char my_number[]="+421908606804";//moj orange
int8_t callstat;
//////////////////////////////////////////////////////////////////
#include <Streaming.h>
#include <RTC.h>
#include <String.h>
#include <dht.h>
dht myDht;
#include <EEPROM.h>
int addr=0;
int eeprom_val;
int addr2=1;
int eeprom_val2;
//my libraries////////////////////////////////////////////////////
#include <Processing.h>
Processing myBattery;
Processing myTemperature;
Processing myHumidity;
#include <Regulation.h>
Regulation myRegulation_R1_1;
#include <Growing.h>
Growing myGrowing;
#include <Dioda.h>
Dioda myDioda;
/////////////////////////////////////////////////////////////////
unsigned long prev_step_gsm;
unsigned long prev_serial_step;
unsigned long prev_dht_step;
int security_status=0;
uint8_t controll_method_irrigation=0;//2..automatic
uint8_t controll_method_main_light=0;//2..automatic
uint8_t controll_method_second_light=0;//2..automatic
uint8_t controll_method_stykac=0;//2..automatic
uint8_t controll_method_stykac2=0;//2..automatic
////////////////////////////////////////////////////////////////////
void setup() {	///////////////////////////////////////////////////
	eeprom_val=EEPROM.read(addr);
	eeprom_val++;
	EEPROM.write(addr,eeprom_val);
		
	fonaSerial->begin(4800);	
	if (! fona.begin(*fonaSerial)) {
		while (1);
	}		
	pinMode(3,INPUT);//DHT input pin
	Serial.begin(115200);
	fona.enableGPRS(false);
	prev_step_gsm=millis();
	prev_serial_step=millis();
	prev_dht_step=millis();
	if (!RTC.read()) {
			Serial.println("Read date error: is time set?");
	} else {}	
}//setup///////////////////////////////////////////////////////////
void loop() {//////////////////////////////////////////////////////
	myDioda.blink();
	myBattery.getProcessing(200,battery_measurement());//step=1000ms. analyzing battery voltage, derivation, etc.
	myRegulation_R1_1.getRegulation(controll_method_stykac, myBattery.getAverage(),R1_1, 14.2,180000, 13.4, 500, 13.5);//stykac
	myGrowing.getWattering(controll_method_irrigation);	
	//myGrowing.getMainLight(controll_method_main_light);
	//myGrowing.getSecondLight(controll_method_second_light,myBattery.getAverage(), 14.1,600000, 13.2, 6000, 13.4);
	gsm_communication();
	serial_comunication();
	dht_measurement();	
}//loop///////////////////////////////////////////////////////////
void flushSerial() {
  while (Serial.available())
    Serial.read();
}
uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}
char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}
void gsm_communication(){
	if ((millis() - prev_step_gsm > 60000)){//gsm processing	
		 prev_step_gsm=millis(); 
		 
		//security call alarm////////////////////////////////////////////
		switch(security_status){
			case 0:			
				if(analogRead(I0_4)<200){
					prev_step_gsm=millis();	
					security_status=1;
				}
			break;
			case 1:
				if(analogRead(I0_4)<200){
					fona.callPhone(my_number);	
					security_status=2;
				}	
				else{security_status=0;}
			break;
			case 2:
			//alarm
			
			break;
			default:
			break;
		}			
		//sms report///////////////////////////////////////////////////
		callstat = fona.getCallStatus();
		Serial<<"Fona Call Staus "<<callstat<<endl;
		if(callstat==3){
			// hang up!
			if (! fona.hangUp()) {
				Serial.println(F("Failed hang up"));
			} else {
			Serial.println(F("OK!"));
			}
			security_status=0;
			report_sms();
		}
		
		// read an SMS///////////////////////////////////////////////////////
		char replybuffer[255];
        uint8_t smsn = 1	;//readnumber();

        // Retrieve SMS sender address/phone number.		
		if (! fona.getSMSSender(smsn, replybuffer, 250)) {
          Serial.println("Failed!");
        }
        Serial.print(F("FROM: ")); 
		Serial.println(replybuffer);
		
		if((replybuffer[12]=='4' && replybuffer[11]=='0' && replybuffer[10]=='8') || (replybuffer[12]=='6' && replybuffer[11]=='9' && replybuffer[10]=='6')){//'+421908606804'
			Serial<<"Number was accepted"<<endl;
			// Retrieve SMS value.
			uint16_t smslen;        
			if (! fona.readSMS(smsn, replybuffer, 250, &smslen)) { // pass in buffer and max len!
			  Serial.println("Failed!");
			  
			}
			Serial.print(F("Text: "));
			Serial.println(replybuffer);
	
			if(replybuffer[0]=='$'){//my sms
				controll_method_irrigation = atoi(&replybuffer[10]);
				controll_method_stykac=atoi(&replybuffer[19]);
				Serial<<"New Controll Method ="<<controll_method_irrigation<<endl;
			    // delete an SMS///////delete my also sms
				Serial.print(F("Delete #"));
				if (fona.deleteSMS(smsn)) {
				  Serial.println(F("OK!"));
				} else {
				  Serial.println(F("Couldn't delete"));
				}
			}			
		}		
		// delete an SMS
		Serial.print(F("Clear SMS possition #1"));
		if (fona.deleteSMS(smsn)) {
		  Serial.println(F("OK!"));
		} else {
		  Serial.println(F("Couldn't delete"));
		}				
		
		
		
		

		
	}	
}
void dht_measurement(){	
	if ((millis() - prev_dht_step > 30000)){///timer, ovladanie cez serial
		prev_dht_step = millis();
		int chk=myDht.read21(3);
		switch (chk){
		case DHTLIB_OK:
				myTemperature.getProcessing(60000,myDht.temperature);
				myHumidity.getProcessing(60000,myDht.humidity);		
			//temp_dht=myDht.temperature;		
			//hum_dht=myDht.humidity;			
		break;
		default:			
			Serial.println("DHT read error");
			//digitalWrite(error_pin,HIGH);
		break;	  
		}	
	}//dht step
}
float battery_measurement(){
	float battery_voltage=analogRead(I1_5)*24.3/794.0/2.0;		
	return battery_voltage;
}
void fast_serial_report(){
	Serial<<RTC.getSecond()<<"	"<< myBattery.getAverage()<<endl;
}
void serial_report_stlpcovy(){	
	
		
		if (!RTC.read()) {
			Serial.println("Read date error: is time set?");
			//digitalWrite(error_pin,HIGH);
		} else {    }
		Serial<<RTC.getHour()<<":"<<RTC.getMinute()<<":"<<RTC.getSecond();
		
		Serial<<"     "<< myBattery.getAverage()<<",  ";
		Serial<<"     "<< myBattery.getDerivation()<<",    ";	
		
		Serial<<" "<<myTemperature.getAverage()<<"  "<<myHumidity.getAverage();
		Serial<<"  "<<analogRead(I0_4)<<"  ";
		
}
void serial_comunication(){
	if ((millis() - prev_serial_step > 1000)){///timer, ovladanie cez serial
		prev_serial_step = millis();
		//read serial input///////////////////////////////////////////////////////
		char serial_input;
		serial_input=Serial.read();	
		//Serial.println(serial_input);
		switch(serial_input){
			case 'i': //irrigation
				Serial.println("Define control method for irrigation");
				flushSerial();
				controll_method_irrigation=readnumber();
			break;
			case 's': //security
				Serial.println("Define controll method for stykac");
				flushSerial();
				controll_method_stykac=readnumber();
				//Serial.println("DEfine controll method for stykac");
			break;
			case 'l': //main light
				Serial.println("Define controll method for main light");
				flushSerial();
				controll_method_main_light=readnumber();
				//Serial.println("DEfine controll method for stykac");
			break;
			case 'L': //irrigation
				Serial.println("Define controll method for second light");
				flushSerial();
				controll_method_second_light=readnumber();
				//Serial.println("DEfine controll method for stykac");
			break;			
			default:
			break;			
		}//switch serial input
		//serial output////////////////////////////////////////////////////////////
		if (!RTC.read()) {
				Serial.println("Read date error: is time set?");
				//digitalWrite(error_pin,HIGH);
			} else {    }
		Serial<<endl<<RTC.getHour()<<":"<<RTC.getMinute()<<":"<<RTC.getSecond()<<endl;
		Serial<<"Voltage="<< myBattery.getAverage()<<",  "<<endl;
		Serial<<"Max Voltage="<< myBattery.getMaxValue()<<",  "<<endl;
		Serial<<"Min Voltage="<< myBattery.getMinValue()<<",  "<<endl;
		Serial<<"Restart number="<< eeprom_val<<",  "<<endl;
		Serial<<"Switch counter="<<myRegulation_R1_1.getCounter()<<endl;
		Serial<<"Controll method Irrigation="<<controll_method_irrigation<<endl;
		Serial<<"Controll method stykac="<<controll_method_stykac<<endl;
		Serial<<"Controll method Main Light="<<controll_method_main_light<<endl;
		Serial<<"Controll method Second Light="<<controll_method_second_light<<endl;
		Serial<<"Security Status="<<security_status<<endl;
	}//end of step	
}
void report_sms(){
			String string="";			
			string+="V=";string+=myBattery.getAverage();			
			string+="V min=";
			string+=myBattery.getMinValue();
			string+="V max=";
			string+=myBattery.getMaxValue();
			string+=" Temp=";
			string+=myTemperature.getAverage();//temp_dht;
			string+=" Temp max=";
			string+=myTemperature.getMaxValue();//temp_dht;
			string+=" Temp min=";
			string+=myTemperature.getMinValue();//temp_dht;
			string+=" Hum=";			
			string+=myHumidity.getAverage();//hum_dht;
			string+="%";
			string+=" SecStat=";
			string+=security_status;
			
			string+=" IrrCon=";
			string+=controll_method_irrigation;
			
			string+=" IrrStat=";
			string+=myGrowing.getWatteringStatus();
			
			string+=" StykCon=";
			string+=controll_method_stykac;		

			string+=" SunMcon=";
			string+=controll_method_main_light;	
			
			string+=" SunScon=";
			string+=controll_method_second_light;	
			
			string+=" SwitchCount=";
			string+=myRegulation_R1_1.getCounter();
							
			string+=" Reset=";			
			string+=eeprom_val;			
			
			char report_buf[150];	
			string.toCharArray(report_buf,150);
			Serial.println(report_buf);			
			if (fona.sendSMS(my_number, report_buf)) {
				Serial.println(F("Failed"));
				//digitalWrite(error_pin,HIGH);  
				}else {
				Serial.println(F("Sent!"));
			}		
			
}
