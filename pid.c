//------------------------------------------ class PID algoritm to keep the temperature -----------------------
/*  The PID algoritm
 *  Un = Kp*(Xs - Xn) + Ki*summ{j=0; j<=n}(Xs - Xj) + Kd(Xn - Xn-1),
 *  Where Xs - is the setup temperature, Xn - the temperature on n-iteration step
 *  In this program the interactive formulae is used:
 *    Un = Un-1 + Kp*(Xn-1 - Xn) + Ki*(Xs - Xn) + Kd*(Xn-2 + Xn - 2*Xn-1)
 *  With the first step:
 *  U0 = Kp*(Xs - X0) + Ki*(Xs - X0); Xn-1 = Xn;
 */
class PID {
  public:
    PID(void) {
      Kp = 768;
      Ki =  30;
      Kd = 196;
    }
    void resetPID(int temp = -1);               // reset PID algoritm history parameters
    // Calculate the power to be applied
    int reqPower(int temp_set, int temp_curr);
    int changePID(byte p, int k);
  private:
    void  debugPID(int t_set, int t_curr, long kp, long ki, long kd, long delta_p);
    int   temp_h0, temp_h1;                     // previously measured temperature
    int   temp_diff_iterate;                    // The temperature difference to start iterate process
    bool  pid_iterate;                          // Whether the iterative process is used
    long  i_summ;                               // Ki summary multiplied by denominator
    long  power;                                // The power iterative multiplied by denominator
    long  Kp, Ki, Kd;                           // The PID algorithm coefficients multiplied by denominator
    const byte denominator_p = 8;               // The common coefficeient denominator power of 2 (8 means divide by 256)
};

void PID::resetPID(int temp) {
  temp_h0 = 0;
  power  = 0;
  i_summ = 0;
  pid_iterate = false;
  temp_diff_iterate = 30;//(temp + 5) / 10;
  if (temp_diff_iterate < 30) temp_diff_iterate = 30;
  if ((temp > 0) && (temp < 1000))
    temp_h1 = temp;
  else
    temp_h1 = 0;
}

int PID::changePID(byte p, int k) {
  switch(p) {
    case 1:
      if (k >= 0) Kp = k;
      return Kp;
    case 2:
      if (k >= 0) Ki = k;
      return Ki;
    case 3:
      if (k >= 0) Kd = k;
      return Kd;
    default:
      break;
  }
  return 0;
}

int PID::reqPower(int temp_set, int temp_curr) {
  if (temp_h0 == 0) {
    // When the temperature is near the preset one, reset the PID and prepare iterative formulae
    if ((temp_set - temp_curr) < temp_diff_iterate) {
      if (!pid_iterate) {
        pid_iterate = true;
        power = 0;
        i_summ = 0;
      }
    }
    i_summ += temp_set - temp_curr;             // first, use the direct formulae, not the iterate process
    power = Kp*(temp_set - temp_curr) + Ki*i_summ;
    // If the temperature is near, prepare the PID iteration process
  } else {
    long kp = Kp * (temp_h1 - temp_curr);
    long ki = Ki * (temp_set - temp_curr);
    long kd = Kd * (temp_h0 + temp_curr - 2*temp_h1);
    long delta_p = kp + ki + kd;
    //debugPID(temp_set, temp_curr, kp, ki, kd, delta_p);
    power += delta_p;                           // power keeped multiplied by denominator!
  }
  if (pid_iterate) temp_h0 = temp_h1;
  temp_h1 = temp_curr;
  long pwr = power + (1 << (denominator_p-1));  // prepare the power to delete by denominator, roud the result
  pwr >>= denominator_p;                        // delete by the denominator
  return int(pwr);
}

void PID::debugPID(int t_set, int t_curr, long kp, long ki, long kd, long delta_p) {
  Serial.print(t_set-t_curr); Serial.print(": ");
  Serial.print("[ "); Serial.print(temp_h0);
  Serial.print(", "); Serial.print(temp_h1);
  Serial.print(", "); Serial.print(t_curr);
  Serial.print(" ] kp = "); Serial.print(kp);
  Serial.print(", ki = "); Serial.print(ki);
  Serial.print(", kd = "); Serial.print(kd);
  Serial.print("; DP = "); Serial.println(delta_p);
}
