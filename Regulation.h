class Regulation{
	public:
	unsigned long prev_time1;
	unsigned long prev_time2;
	int status=0;
	String time_on;
	String time_off;
	void getRegulation(uint8_t controll_method,float battery_voltage, int pin, float on_voltage, unsigned long waiting2, float off_voltage, unsigned long waiting1, float off2_voltage);  //,int *status
	String getTime_off();
	String getTime_on();
	int getStatus();
	int counter=0;
	int getCounter();
	
  };
void Regulation::getRegulation(uint8_t controll_method, float battery_voltage, int pin, float on_voltage, unsigned long waiting2, float off_voltage, unsigned long waiting1, float off2_voltage){//, int *status //on_voltage=13.9, off_voltage=12.5
	
	switch(controll_method){
		case 0:
			status=0;
			digitalWrite(pin,LOW);
			//Serial<<"Pin"<<pin<<" is off"<<endl;
		break;
		case 1:
			status=1;
			digitalWrite(pin,HIGH);
			//Serial<<"Pin"<<pin<<" is on"<<endl;		
		break;
		case 2:
			switch(status){
			case 0:
				if(battery_voltage>on_voltage){
					digitalWrite(pin,HIGH);
					status=1;}
			break;
			case 1:
				status=2;
				time_on="";time_on+=RTC.getHour();time_on+=":";time_on+=RTC.getMinute();
				counter++;
			break;
			case 2:
				if(battery_voltage<off_voltage){
					prev_time1=millis();				
					status=22;
				}
			break;
			case 22:
				if ((millis() - prev_time1 > waiting1)){
					 
					if(battery_voltage<off2_voltage){
							status=3;
							digitalWrite(pin,LOW);
					}				
				}
			break;		
			case 3:
				status=4;
				time_off="";time_off+=RTC.getHour();time_off+=":";time_off+=RTC.getMinute();
			break;
			case 4:
				if(battery_voltage>on_voltage){
					status=5;
					prev_time2=millis();
				}
			break;
			case 5:
				if ((millis() - prev_time2 > waiting2)){
					status=0;            
				}
			break;
			}
		
		
		
		break;
		
		
		
	}
	
	
	
	
	
		

	}
String Regulation::getTime_on(){	
	return time_on;
}
String Regulation::getTime_off(){	
	return time_off;
}
int Regulation::getStatus(){
	return status;	
}
int Regulation::getCounter(){
	
	return counter;
	
	
}