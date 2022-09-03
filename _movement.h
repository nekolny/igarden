class _movement{
  public:
		unsigned long prev_mov=millis();
		unsigned long prev_mov_off=millis();
		int movement_status=0;
		byte regulation_status=0;	
		int movement_detection(int pin);	
		int detection_counter=0;
};



int _movement::movement_detection(int pin){
	switch (movement_status){
		case 0:
			if(analogRead(pin)>200){movement_status=1;}
		break;
		case 1:
			
			if(detection_counter<10){
				while (!Serial);  pinMode(FONA_KEY, OUTPUT);  digitalWrite(FONA_KEY, HIGH);
				fonaSerial->begin(4800);
				if (! fona.begin(*fonaSerial)) {    
					//Serial.println(F("Couldn't find FONA"));  
				while (1);}
				fona.callPhone(my_number);
				prev_mov=millis();
				movement_status=2;
			}
		break;
		case 2:
			fona.callPhone(my_number);
			if(millis()-prev_mov>30000){
				fona.hangUp();
				detection_counter++;
				movement_status=3;
				prev_mov_off=millis();
			}
		break;
		case 3://waiting to next movement
			if(millis()-prev_mov_off>120000){
				movement_status=0;
			}
		break;
		
		default:
		fona.hangUp();
		movement_status=0;
		break;
	}
return movement_status;
}

