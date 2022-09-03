//calculate avere value of a signal, derivation of a signal and other parameters

class Processing{
  //private:
  public:
	float measured_value_prev=0.0;
	float measured_value=0.0;
	float sum=0.0;
	unsigned long i=0;
	float derivation=0.0;
	int status=1;//1...charging or full charged, 0...discharging or low
	float measured_value_limit=14.0;
	float min_measured_value=1000.0;
	float max_measured_value=-1000.0;	
	unsigned long prev_time=millis();		
	
	void getProcessing(int interval, float value);//interval [ms], default is 1000
	float getAverage();
	float getDerivation();
	int getStatus();	
	float getMinValue();
	float getMaxValue();
  };
  
void Processing::getProcessing(int interval, float value){
	sum=sum+value;
	i++;
	if ((millis() - prev_time > interval)){	
		prev_time=millis();
		measured_value_prev=measured_value;
		measured_value=sum/i;
		derivation=(measured_value-measured_value_prev)*1000.0;///(float)interval;
		sum=0.0;
		i=0;	
		if(derivation>0){
			status=1;		
		}
		else{
			status=0;
		}
		if(measured_value<min_measured_value){min_measured_value=measured_value;}
		if(measured_value>max_measured_value){max_measured_value=measured_value;}
		
		//print test values/////////////////////////////////////////////
		//Serial<<measured_value<<" "<<derivation<<endl;
	}	
	
}
float Processing::getAverage(){
	return measured_value;
}
float Processing::getDerivation(){
	return derivation;
}
int Processing::getStatus(){
	return status;	
}

float Processing::getMinValue(){	
	return min_measured_value;	
}
float Processing::getMaxValue(){	
	return max_measured_value;	
}