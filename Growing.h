class Growing{
	public:
	unsigned long prev_time;
	//uint8_t status=0;
	uint8_t wattering_status=0;
	void getWattering(byte controll_method);	
	uint8_t getWatteringStatus();
	
	void getMainLight(uint8_t controll_method_main_light);
	uint8_t getMainLightStatus();
	uint8_t main_light_status=0;
	void getSecondLight(uint8_t controll_method_second_light, float battery_voltage, float on_voltage, unsigned long waiting2, float off_voltage, unsigned long waiting1, float off2_voltage);
	uint8_t getSecondLightStatus();
	uint8_t second_light_status=0;
	unsigned long prev_time1;
	unsigned long prev_time2;
  };
void Growing::getWattering(uint8_t controll_method){//0..manual off, 1.. manual ON, 2..automatic.
	switch(controll_method){
		case 0:
			digitalWrite(R0_1,LOW);	
			wattering_status=0;		
			//Serial<<"Wattering is OFF"<<endl;
		break;
		case 1:
			digitalWrite(R0_1,HIGH);
			wattering_status=1;
			//Serial<<"Wattering is ON"<<endl;	
		break;
		case 2:
			if (!RTC.read()) {	Serial.println("Read date error: is time set?");} 
			else {}
			switch(wattering_status){
				case 0:
					if(RTC.getHour()>=20 && RTC.getHour()<21){
					digitalWrite(R0_1,HIGH);		
					Serial<<"Wattering is ON"<<endl;
					wattering_status=1;
					}				
				break;				
				case 1:
					if(RTC.getHour() >=21 ){
					digitalWrite(R0_1,LOW);		
					Serial<<"Wattering is OFF"<<endl;
					wattering_status=0;
					}
				break;			
			}
		break;
	} 
}
uint8_t Growing::getWatteringStatus(){
	return wattering_status;	
}
void Growing::getMainLight(uint8_t controll_method_main_light){
	switch(controll_method_main_light){
		case 0:
			digitalWrite(R0_2,LOW);		
			//Serial<<"Light is OFF"<<endl;
			main_light_status=0;		
		break;		
		case 2:
			switch(main_light_status){
					case 0:
						if(RTC.getHour()>=8 && RTC.getHour() <20  ){
						digitalWrite(R0_2,HIGH);		
						Serial<<"Light is ON"<<endl;
						main_light_status=1;
						}				
					break;				
					case 1:
						if(RTC.getHour() >=20 ){
						digitalWrite(R0_2,LOW);		
						Serial<<"Light is OFF"<<endl;
						main_light_status=0;
						}
					break;			
			}
		break;		
	}		
}
uint8_t Growing::getMainLightStatus(){
	return main_light_status;
}
void Growing::getSecondLight(uint8_t controll_method_second_light, float battery_voltage, float on_voltage, unsigned long waiting2, float off_voltage, unsigned long waiting1, float off2_voltage){
	
	switch(controll_method_second_light){
		case 0:
			analogWrite(Q1_0,0);
			//Serial<<"Light is OFF"<<endl;
			second_light_status=0;
		break;
		case 2:	
		
			switch(second_light_status){
				case 0:
					if(battery_voltage>on_voltage && RTC.getHour()>=8 && RTC.getHour()<20 && main_light_status==1){
						analogWrite(Q1_0,255);
						second_light_status=1;}
				break;
				case 1:
					second_light_status=2;
				break;
				case 2:
					if(battery_voltage<off_voltage){
						prev_time1=millis();				
						second_light_status=22;
					}
				break;
				case 22:
					if ((millis() - prev_time1 > waiting1)){
						 
						if(battery_voltage<off2_voltage){
								second_light_status=3;
								analogWrite(Q1_0,0);
						}
						else{
							second_light_status=1;
						}
						
					}					
				break;		
				case 3:
					second_light_status=4;
				break;
				case 4:
					if(battery_voltage>on_voltage){
						second_light_status=5;
						prev_time2=millis();
					}
				break;
				case 5:
					if ((millis() - prev_time2 > waiting2)){
						second_light_status=0;            
					}
				break;
			}
		break;
	}	
}
uint8_t Growing::getSecondLightStatus(){
	return second_light_status;
	
}
 