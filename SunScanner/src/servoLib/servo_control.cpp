#include "servoLib/servo_control.h"

/* Costruttore */
ServoController::ServoController(int pin_az, int pin_el, int current_p, int voltage_p):
	servoAz(360,pin_az),
	servoEl(180,pin_el){
	current_pin = current_p;
	voltage_pin = voltage_p;
	pinMode(current_pin,INPUT);
	pinMode(voltage_pin,INPUT);
}

/* point to sun automatically */
void ServoController::auto_positioning(tm* time_info_ptr, double Lat, double Lon, double Alt, double compass_val, double* Az, double* El){
	int azpos;
	SolarAzEl(time_info_ptr, Lat, Lon, Alt, Az, El);
	azpos=*(Az);

	/* compass adjustments */
	if(compass_val>180){
		azpos+=(360-compass_val);
	}else{
		azpos-=compass_val;
	}
	
  	/* these adjustments work if the 0 angle of the servoAz face same direction of compass */
	servoAz.set_servo(azpos);
	servoEl.set_servo(*(El));
}

/* ADC read of voltage and current from solar panel */
double ServoController::get_voltage(){
	int voltage = analogRead(voltage_pin);
	return(voltage);
}

/* Function that point at the most power efficient point  */
/* step => how many degrees each servo move per iteration */
/* n_iter => how many step are taken in each direction */
void ServoController::max_power_pos(double step, int n_iter){
	int pow, max_pow_az, max_pow_el;
	int i;
	double max_az, max_el; 
	double initial_el = servoEl.get_alpha();  //starting angle of elevation servo
	double initial_az = servoAz.get_alpha();  //starting angle of azimuth servo

	max_az = initial_az;
	max_el = initial_el;
	max_pow_az = max_pow_el = get_voltage()*get_current();
	for(i=0;i<n_iter;++i){
		double az = servoAz.get_alpha();
		servoAz.set_servo(az-step);  //moving servo of step degree
		pow=get_voltage()*get_current();  //calculating power
		if(pow>max_pow_az){  //finding the most powerful position
			max_pow_az = pow;
			max_az = az-step;
		}
	}
	
	servoAz.set_servo(initial_az);
	for(i=0;i<n_iter;++i){
		double az = servoAz.get_alpha();
		servoAz.set_servo(az+step); 
		pow=get_voltage()*get_current();
		if(pow>max_pow_az){
			max_pow_az = pow;
			max_az = az+step;
		}
	}
	
	servoAz.set_servo(initial_az);
	for(i=0;i<n_iter;++i){
		double el = servoEl.get_alpha();
		servoEl.set_servo(el+step);
		pow=get_voltage()*get_current();
		if(pow>max_pow_el){
			max_pow_el = pow;
			max_el = el+step;
		}
	}
	
	servoEl.set_servo(initial_el);
	for(i=0;i<n_iter;++i){
		double el = servoEl.get_alpha();
		servoEl.set_servo(el-step);
		pow=get_voltage()*get_current();
		if(pow>max_pow_el){
			max_pow_el = pow;
			max_el = el-step;
		}
	}
	delay(500);
	
	servoAz.set_servo(max_az);
	servoEl.set_servo(max_el);
}
