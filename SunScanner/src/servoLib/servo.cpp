#include "servoLib/servo.h"

/* In order to make everything work, after creating a servo, set_pins() must be called */
Servo::Servo(int max_ag, int p){
	max_alpha = max_ag;
	pin = p;
	pinMode(pin,OUTPUT);
	//ledcSetup(pwm.channel,50,pwm.resolution);
	//ledcAttachPin(pwm.pin,pwm.channel);
	set_servo(0);
}

/* return rispectively alpha and duty_alpha */
double Servo::get_alpha(){
	return alpha;
}
int Servo::get_duty_alpha(){
	return duty_alpha;
}

/* Calculate how long pwm pin should be on for a given angle */
int Servo::duty_calc(double angle){
	int t_on = (angle * 2000 / max_alpha) + 500; //calculating the amount of ms signal must be high for pwm
	return t_on; 
}

/* put the anggle back in range (0 - max_alpha) */
double Servo::rebound(int angle){
	angle = angle < 0 ? 0 : angle;
	angle = angle > max_alpha ? max_alpha : angle;
	return angle;
}

/* Calculate how many iteration of pwm are necessary to move servo */
int Servo::calc_iter_PWM(double angle1, double angle2){
	int ms = angle1>angle2 ? ceil(angle1-angle2) : ceil(angle2-angle1);
	ms = ms < 25 ? 25 : ms;
	return ms;
}

/* Set alpha and calculate duty_alpha */
void Servo::set_servo(double angle){
	double starting_alpha=alpha;
	int ms;
	
	alpha = rebound(angle);
	duty_alpha = duty_calc(alpha);
	ms = calc_iter_PWM(starting_alpha,alpha);
	for(int i = 0; i < ms; ++i){
		digitalWrite(pin,HIGH);
		delayMicroseconds(duty_alpha);
		digitalWrite(pin,LOW);
		delayMicroseconds(20000-duty_alpha);
	}
}


