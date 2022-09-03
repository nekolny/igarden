class Dioda{

	public:	
	#define d1_pin Q0_0
	#define d2_pin Q0_1
	#define d3_pin Q0_2	
	unsigned long step=100;
	unsigned long prev_step=millis();
	byte status=0;


	int blink(){
		
			
		
		if ((millis() - prev_step > step)){
			
			prev_step=millis();
			step=random(200);
			switch(status){
			case 0:
				analogWrite(d3_pin,0);
				analogWrite(d1_pin,10);
				status=1;			
			break;
			case 1:
				analogWrite(d1_pin,0);
				analogWrite(d2_pin,10);
				status=2;
			break;
			case 2:
				analogWrite(d2_pin,0);
				analogWrite(d3_pin,10);
				status=0;
			break;		
			}			
		}
		
		
		
		
	}
	
	
	
	
	
	
	
	
};
