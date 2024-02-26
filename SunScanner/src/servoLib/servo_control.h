#ifndef __SERVO_CONTROL_H__
#define __SERVO_CONTROL_H__

#include "servo.h"
#include "Sun_az_alt.h"

class ServoController{
private:
	int voltage_pin, current_pin;
public:
  Servo servoAz, servoEl;

  
	ServoController(int pin_az, int pin_el, int current_p, int voltage_p);
	
	/* Function that point at the most power efficient point  */
	/* step => how many degrees each servo move per iteration */
	/* n_iter => how many step are taken in each direction */
  /* step_delay_ms => delay betwen */
	void max_power_pos(double step, int n_iter);

	/* ADC read of voltage and current from solar panel */
	double get_voltage();
	double get_current();
	
	/* point to sun automatically */
	void auto_positioning(tm* time_info_ptr, double Lat, double Lon, double Alt, double* Az, double* El);
};

#endif

