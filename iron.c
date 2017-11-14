//------------------------------------------ class soldering iron ---------------------------------------------
class IRON : protected PID {
  public:
    IRON(byte heater_pin, byte sensor_pin) {
      hPIN = heater_pin;
      sPIN = sensor_pin;
      on = false;
      unit_celsius = true;
      fix_power = false;
      unit_celsius = true;
      no_iron = true;
    }
    void     init(uint16_t t_max, uint16_t t_min);
    void     switchPower(bool On);
    bool     isOn(void)                         { return on; }
    bool     isCold(void)                       { return (h_temp.last() < temp_cold); }
    bool     noIron(void)                       { return no_iron; }
    void     setTempUnits(bool celsius)         { unit_celsius = celsius; }
    bool     getTempUnits(void)                 { return unit_celsius; }
    uint16_t getTemp(void)                      { return temp_set; }
    uint16_t getCurrTemp(void)                  { return h_temp.last(); }
    uint16_t tempAverage(void)                  { return h_temp.average(); }
    uint16_t tempDispersion(void)               { return h_temp.dispersion(); }
    uint16_t powerDispersion(void)              { return h_power.dispersion(); }
    byte     getMaxFixedPower(void)             { return max_fixed_power; }
    int      changePID(byte p, int k)           { return PID::changePID(p, k); }
    void     setTemp(int t);                    // Set the temperature to be keeped
    // Set the temperature to be keeped in human readable units (celsius or farenheit)
    void     setTempHumanUnits(int t);
    // Translate internal temperature to the celsius or farenheit
    uint16_t temp2humanUnits(uint16_t temp);
    byte     getAvgPower(void);                 // Average applied power
    byte     appliedPower(void);                // Power applied to the solder [0-100%]
    byte     hotPercent(void);                  // How hot is the iron (used in the idle state)
    void     keepTemp(void);                    // Main solder iron loop
    bool     fixPower(byte Power);              // Set the specified power to the the soldering iron
    bool     used(void);                        // Whether the iron was previously used (Hot)
  private:
    uint16_t   temp(void);                      // Read the actual temperature of the soldering iron
    void       applyPower(void);                // Check the the power limits and apply power to the heater
    FastPWMdac fastPWMdac;                      // Power the irom using fastPWMdac
    uint32_t   checkMS;                         // Milliseconds to measure the temperature next time
    byte       hPIN, sPIN;                      // The heater PIN and the sensor PIN
    int        power;                           // The soldering station power
    byte       actual_power;                    // The power supplied to the iron
    bool       on;                              // Whether the soldering iron is on
    bool       fix_power;                       // Whether the soldering iron is set the fix power
    bool       no_iron;                         // Whether the iron is connected
    bool       unit_celsius;                    // Human readable units for the temparature (celsius or farenheit)
    int        temp_set;                        // The temperature that should be keeped
    bool       iron_checked;                    // Whether the iron works
    int        temp_start;                      // The temperature when the solder was switched on
    uint32_t   elapsed_time;                    // The time elipsed from the start (ms)
    uint16_t   temp_min;                        // The minimum temperature (180 centegrees)
    uint16_t   temp_max;                        // The maximum temperature (400 centegrees)
    HISTORY    h_power;
    HISTORY    h_temp;
    const uint16_t temp_cold   = 340;           // The cold temperature to touch the iron safely
    const uint16_t temp_no_iron = 980;          // Sensor reading when the iron disconnected
    const byte max_power       = 180;           // maximum power to the iron (220)
    const byte max_fixed_power = 120;           // Maximum power in fiexed power mode
    const uint16_t period      = 200;           // The period to check the soldering iron temperature, ms
    const int check_time       = 10000;         // Time in ms to check Whether the solder is heating
    const int heat_expected    = 10;            // The iron should change the temperature at check_time
};

void IRON::setTemp(int t) {
  if (on) resetPID();
  temp_set = t;
}

void IRON::setTempHumanUnits(int t) {
  int temp;
  if (unit_celsius) {
    if (t < temp_minC) t = temp_minC;
    if (t > temp_maxC) t = temp_maxC;
    temp = map(t+1, temp_minC, temp_maxC, temp_min, temp_max);
  } else {
    if (t < temp_minF) t = temp_minF;
    if (t > temp_maxF) t = temp_maxF;
    temp = map(t+2, temp_minF, temp_maxF, temp_min, temp_max);
  }
  for (byte i = 0; i < 10; ++i) {
    int tH = temp2humanUnits(temp);
    if (tH <= t) break;
    --temp;
  }
  setTemp(temp);
}

uint16_t IRON::temp2humanUnits(uint16_t temp) {
  if (!unit_celsius)  return map(temp, temp_min, temp_max, temp_minF, temp_maxF);
  return map(temp, temp_min, temp_max, temp_minC, temp_maxC);
}

byte IRON::getAvgPower(void) {
  int p = h_power.average();
  return p & 0xff;
}

byte IRON::appliedPower(void) {
  byte p = getAvgPower();
  return map(p, 0, max_power, 0, 100);
}

byte IRON::hotPercent(void) {
  uint16_t t = h_temp.average();
  char r = map(t, temp_cold, temp_set, 0, 100);
  if (r < 0) r = 0;
  return r;
}

void IRON::init(uint16_t t_max, uint16_t t_min) {
  pinMode(sPIN, INPUT);
  fastPWMdac.init(hPIN, 8);                     // initialization for 8 bit resolution
  fastPWMdac.analogWrite8bit(0);                // sawtooth output, period = 31.25Khz
  on = false;
  fix_power = false;
  power = 0;
  actual_power = 0;
  checkMS = 0;

  elapsed_time = 0;
  temp_start = analogRead(sPIN);
  iron_checked = false;
  temp_max = t_max; temp_min = t_min;

  resetPID();
  h_power.init();
  h_temp.init();
}

void IRON::switchPower(bool On) {
  on = On;
  if (!on) {
    fastPWMdac.analogWrite8bit(0);
    fix_power = false;
    return;
  }

  resetPID(analogRead(sPIN));
  h_power.init();
  checkMS = millis();
}

void IRON::keepTemp(void) {
  if (checkMS > millis()) return;
  checkMS = millis() + period;

  if (!on) {                                    // If the soldering iron is set to be switched off
    if (!fix_power)
      fastPWMdac.analogWrite8bit(0);            // Surely power off the iron
  }

  int16_t temp = 0;
  if (actual_power > 0)
    fastPWMdac.analogWrite8bit(0);              // switch-off supplied power to be more accurate
  delay(20);
  int16_t t1 = analogRead(sPIN);
  delayMicroseconds(50);
  int16_t t2 = analogRead(sPIN);
  if (actual_power > 0)                         // restore the power after measurement of the temperature
    fastPWMdac.analogWrite8bit(actual_power);

  if (abs(t1 - t2) < 10) {                      // use average of two samples if they are simiral
    t1 += t2 + 1;
    t1 >>= 1;                                   // average of two measurements
    temp = t1;
  } else {                                      // use sample that is near to the previous
    int tprev = h_temp.last();
    if (abs(t1 - tprev) < abs(t2 - tprev))
      temp = t1;
    else
      temp = t2;
  }

  // Check Whether the iron can be heated
  if (!iron_checked) {
    elapsed_time += period;
    if (elapsed_time >= check_time) {
      if ((abs(temp_set - temp) < 100) || ((temp - temp_start) > heat_expected)) {
        iron_checked = true;
      } else {
        switchPower(false);                     // Prevent the iron damage
        elapsed_time = 0;
        temp_start = analogRead(sPIN);
        iron_checked = false;
      }
    }
  }

  // If the power is off and no iron detected, do not put the temperature into the history
  if (!on && !fix_power && (temp > temp_no_iron)) {
    no_iron = true;
  } else {
    no_iron = false;
    h_temp.put(temp);
  }

  // Use PID algoritm to calculate power to be applied
  power = reqPower(temp_set, temp);
  applyPower();
}

void IRON::applyPower(void) {
  int p = power;
  if (p < 0) p = 0;
  if (p > max_power) p = max_power;

  if (h_temp.last() > (temp_set + 8)) p = 0;
  if (p == 0) actual_power = 0;
  if (on) actual_power = p & 0xff;
  h_power.put(p);
  fastPWMdac.analogWrite8bit(actual_power);
}

bool IRON::fixPower(byte Power) {
  if (Power == 0) {                             // To switch off the iron, set the power to 0
    fix_power = false;
    actual_power = 0;
    fastPWMdac.analogWrite8bit(0);
    return true;
  }

  if (Power > max_fixed_power) {
    actual_power = 0;
    return false;
  }

  if (!fix_power) {
    fix_power = true;
    power = Power;
    actual_power = power & 0xff;
  } else {
    if (power != Power) {
      power = Power;
      actual_power = power & 0xff;
    }
  }
  fastPWMdac.analogWrite8bit(actual_power);
  return true;
}

bool IRON::used(void) {
  uint16_t temp = h_temp.last();
  return ((temp > temp_cold) && (temp < temp_no_iron));
}
