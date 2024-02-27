#ifndef __SERVO_H__
#define __SERVO_H__

#include <arduino.h>
#include <math.h>

#define SERVO_SPEED 30  //Number of millisecond to rotate one degree

class Servo{
private:
	double alpha, max_alpha; //variables that rapresent servo angle
	int duty_alpha; //corresponding duty cycles, depends on resolution
	int pin;
public:
	Servo(int max_ag, int p);
	
	/* Return rispectively alpha and duty_alpha */
	double get_alpha();
	int get_duty_alpha();
	
	/* Calculate duty cycle given an angle and pwm resolution */
	int duty_calc(double angle);
	
	/* put the anggle back in range (0 - 360) */
	double rebound(int angle);
	
	/* Calculate how many saconds take for the rotation */
	int calc_iter_PWM(double angle1, double angle2);
	
	/* Set alpha and teta and calculate duty_alpha and duty_teta */
	void set_servo(double angle);
};

#endif

