//------------------------------------------ class lcd DSPLay for soldering iron -----------------------------
class DSPL : protected LiquidCrystal {
  public:
    DSPL(byte RS, byte E, byte DB4, byte DB5, byte DB6, byte DB7) : LiquidCrystal(RS, E, DB4, DB5, DB6, DB7) { }
    void init(void);
    void clear(void) { LiquidCrystal::clear(); }
    void tSet(uint16_t t, bool celsuis);        // Show the temperature set
    void tCurr(uint16_t t);                     // Show The current temperature
    void pSet(byte p);                          // Show the power set
    void tempLim(byte indx, uint16_t temp);     // Show the upper or lower temperature limit
    void timeToOff(byte sec);                   // Show the time to automatic off the iron
    void msgNoIron(void);                       // Show 'No iron' message
    void msgReady(void);                        // Show 'Ready' message
    void msgWorking(void);                      // Show 'Working' message
    void msgOn(void);                           // Show 'On' message
	  void msgOff(void);                          // Show 'Off' message
	  void msgCold(void);                         // Show 'Cold' message
	  void msgFail(void);                         // Show 'Fail' message
	  void msgTune(void);                         // Show 'Tune' message
	  void msgCelsius(void);                      // Show 'Cels.' message
	  void msgFarneheit(void);                    // Show 'Faren.' message
    void msgUpper(void);                        // Show 'setting upper temperature' process
    void msgLower(void);                        // Show 'setting lower temperature' process
    void msgDefault();                          // Show 'default' message (load default configuratuin)
    void msgCancel(void);                       // Show 'cancel' message
    void msgApply(void);                        // Show 'save message'
    void setupMode(byte mode, byte p = 0);      // Show the configureation mode [0 - 2]
    void show(void) {}                          // No animation ever
    void percent(byte Power);                   // Show the percentage
  private:
    bool full_second_line;                      // Whether the second line is full with the message
};

void DSPL::init(void) {
  LiquidCrystal::begin(8, 2);
  LiquidCrystal::clear();
  full_second_line = false;
}

void DSPL::tSet(uint16_t t, bool celsius) {
  char buff[5];
  char units = 'C';
  if (!celsius) units = 'F';
  LiquidCrystal::setCursor(0, 0);
  sprintf(buff, "%3d%c", t, units);
  LiquidCrystal::print(buff);
}

void DSPL::tCurr(uint16_t t) {
  char buff[4];
  LiquidCrystal::setCursor(0, 1);
  if (t < 1000) {
    sprintf(buff, "%3d", t);
  } else {
    LiquidCrystal::print(F("xxx"));
    return;
  }
  LiquidCrystal::print(buff);
  if (full_second_line) {
    LiquidCrystal::print(F("    "));
    full_second_line = false;
  }
}

void DSPL::pSet(byte p) {
  char buff[6];
  sprintf(buff, "P:%3d", p);
  LiquidCrystal::setCursor(0, 0);
  LiquidCrystal::print(buff);
}

void DSPL::tempLim(byte indx, uint16_t temp) {
  char buff[9];
  if (indx == 0) {
    buff[0] = 'u';
    buff[1] = 'p';
  } else {
    buff[0] = 'l';
    buff[1] = 'o';
  }
  sprintf(&buff[2], ": %3d ", temp);
  LiquidCrystal::setCursor(0, 1);
  LiquidCrystal::print(buff);
}

void DSPL::timeToOff(byte sec) {
  char buff[5];
  if (sec >= 1000) return;
  sprintf(buff, " %3d", sec);
  LiquidCrystal::setCursor(4, 0);
  LiquidCrystal::print(buff);
}

void DSPL::msgNoIron(void) {
  LiquidCrystal::setCursor(0, 1);
  LiquidCrystal::print(F("no iron "));
  full_second_line = true;
}

void DSPL::msgReady(void) {
  LiquidCrystal::setCursor(4, 0);
  LiquidCrystal::print(F(" rdy"));
}

void DSPL::msgWorking(void) {
  LiquidCrystal::setCursor(4, 0);
  LiquidCrystal::print(F(" wrk"));
}

void DSPL::msgOn(void) {
  LiquidCrystal::setCursor(4, 0);
  LiquidCrystal::print(F("  ON"));
}

void DSPL::msgOff(void) {
  LiquidCrystal::setCursor(4, 0);
  LiquidCrystal::print(F(" OFF"));
}

void DSPL::msgCold(void) {
  LiquidCrystal::setCursor(0, 1);
  LiquidCrystal::print(F("  cold  "));
  full_second_line = true;
}

void DSPL::msgFail(void) {
  LiquidCrystal::setCursor(0, 1);
  LiquidCrystal::print(F(" Failed "));
}

void DSPL::msgTune(void) {
  LiquidCrystal::setCursor(0, 0);
  LiquidCrystal::print(F("Tune"));
}

void DSPL::msgCelsius(void) {
  LiquidCrystal::setCursor(0, 1);
  LiquidCrystal::print(F("Celsius "));
}

void DSPL::msgFarneheit(void) {
  LiquidCrystal::setCursor(0, 1);
  LiquidCrystal::print(F("Faren.  "));
}

void DSPL::msgUpper(void) {
  LiquidCrystal::setCursor(6, 0);
  LiquidCrystal::print(F("up"));
}

void DSPL::msgLower(void) {
  LiquidCrystal::setCursor(6, 0);
  LiquidCrystal::print(F("lo"));
}

void DSPL::msgDefault() {
  LiquidCrystal::setCursor(0, 1);
  LiquidCrystal::print(F(" default"));
}

void DSPL::msgCancel(void) {
  LiquidCrystal::setCursor(0, 1);
  LiquidCrystal::print(F(" cancel "));
}

void DSPL::msgApply(void) {
  LiquidCrystal::setCursor(0, 1);
  LiquidCrystal::print(F(" save   "));
}

void DSPL::setupMode(byte mode, byte p) {
  char buff[5];
  LiquidCrystal::clear();
  LiquidCrystal::print(F("setup"));
  LiquidCrystal::setCursor(1,1);
  switch (mode) {
    case 0:
	  LiquidCrystal::print(F("off:"));
    if (p > 0) {
      sprintf(buff, "%2dm", p);
      LiquidCrystal::print(buff);
    } else {
      LiquidCrystal::print(" NO");
    }
	  break;
    case 1:
      LiquidCrystal::print(F("units"));
      LiquidCrystal::setCursor(7,1);
      if (p)
        LiquidCrystal::print("C");
      else
        LiquidCrystal::print("F");
      break;
    case 2:
      LiquidCrystal::print(F("tune"));
      break;
  }
}

void DSPL::percent(byte Power) {
  char buff[6];
  sprintf(buff, " %3d%c", Power, '%');
  LiquidCrystal::setCursor(3, 1);
  LiquidCrystal::print(buff);
}
