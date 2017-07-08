#include "PID.h"

using namespace std;

/*
* TODO: Complete the PID class.
*/

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd) {
	this->Kp_default=Kp;
  this->Ki_default=Ki;
  this->Kd_default=Kd;

  this->Kp=Kp;
  this->Ki=Ki;
  this->Kd=Kd;

  this->p_error = 0;
  this->i_error = 0;
  this->d_error = 0;
  this->prev_cte = 0;
}

void PID::UpdateParameters(double speed){
	this->Kp= this->Kp_default - 0.000 * speed;
  this->Ki= this->Ki_default + 0.0000 * speed;
  this->Kd= this->Kd_default + 0.00 * speed;
}

void PID::UpdateError(double cte) {

	this->p_error = cte;
  this->i_error += cte;
  this->d_error = cte - this->prev_cte;
  this->prev_cte = cte;
  
}

double PID::TotalError() {
	return this->Kp * this->p_error + this->Ki_default * this->i_error + this->Kd * this->d_error;
}