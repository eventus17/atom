//------------------------------------------ class FastPWDdac --------------------------------------------------
/*
FastPWMdac
Copyright (C) 2015  Albert van Dalen http://www.avdweb.nl
*/

class FastPWMdac {
  public:
    void init(byte _timer1PWMpin, byte resolution);
    void analogWrite8bit(byte value8bit);
    void analogWrite10bit(int value10bit);
  private:
    byte timer1PWMpin;
};


void FastPWMdac::init(byte _timer1PWMpin, byte resolution){
  timer1PWMpin = _timer1PWMpin;
  if(resolution == 8) Timer1.initialize(32);
  if(resolution == 10) Timer1.initialize(128);
  Timer1.pwm(timer1PWMpin, 0);                  // dummy, required before setPwmDuty()
}

void FastPWMdac::analogWrite8bit(byte value8bit){
  Timer1.setPwmDuty(timer1PWMpin, value8bit*4); // faster than pwm()
}

void FastPWMdac::analogWrite10bit(int value10bit) {
  Timer1.setPwmDuty(timer1PWMpin, value10bit); // faster than pwm()
}
