//---------------------------------------- class pidSCREEN [tune the PID coefficients] -------------------------
class pidSCREEN : public SCREEN {
  public:
    pidSCREEN(IRON* Iron, ENCODER* ENC) {
      pIron = Iron;
      pEnc = ENC;
    }
    virtual void init(void);
    virtual SCREEN* menu(void);
    virtual SCREEN* menu_long(void);
    virtual void show(void);
    virtual void rotaryValue(int16_t value);
  private:
    IRON* pIron;                                // Pointer to the iron instance
    ENCODER* pEnc;                              // Pointer to the rotary encoder instance
    byte mode;                                  // Which temperature to tune [0-3]: select, Kp, Ki, Kd
    uint32_t update_screen;                     // The time in ms when the screen should be updated
    int temp_set;
    const uint16_t period = 500;                // The period the screen to be updated
};

void pidSCREEN::init(void) {
  temp_set = pIron->getTemp();
  mode = 0;                                     // select the element from the list
  pEnc->reset(1, 1, 4, 1, 1, true);             // 1 - Kp, 2 - Ki, 3 - Kd, 4 - temp
  Serial.println("Select the coefficient (Kp)");
  update_screen = 0;
}

void pidSCREEN::rotaryValue(int16_t value) {
  if (mode == 0) {                              // No limit is selected, list the menu
    Serial.print("[");
    for (byte i = 1; i < 4; ++i) {
      int k = pIron->changePID(i, -1);
      Serial.print(k, DEC);
      if (i < 3) Serial.print(", ");
    }
    Serial.print("]; ");
    switch (value) {
      case 1:
        Serial.println("Kp");
        break;
      case 2:
        Serial.println("Ki");
        break;
      case 4:
        Serial.println(F("Temp"));
        break;
      case 3:
      default:
       Serial.println("Kd");
       break;
    }
  } else {
    switch (mode) {
      case 1:
        Serial.print(F("Kp = "));
        pIron->changePID(mode, value);
        break;
      case 2:
        Serial.print(F("Ki = "));
        pIron->changePID(mode, value);
        break;
      case 4:
        Serial.print(F("Temp = "));
        temp_set = value;
        pIron->setTemp(value);
        break;
      case 3:
      default:
        Serial.print(F("Kd = "));
        pIron->changePID(mode, value);
        break;
    }
    Serial.println(value);
  }
}

void pidSCREEN::show(void) {
  if (millis() < update_screen) return;
  update_screen = millis() + period;
  if (pIron->isOn()) {
    char buff[60];
    int temp    = pIron->getCurrTemp();
    uint16_t td = pIron->tempDispersion();
    uint16_t pd = pIron->powerDispersion();
    sprintf(buff, "%3d: td = %3d, pd = %3d --- ", temp_set - temp, td, pd);
    Serial.println(buff);
    //if ((temp_set - temp) > 30) Serial.println("");
  }
}

SCREEN* pidSCREEN::menu(void) {                 // The rotary button pressed
  if (mode == 0) {                              // select upper or lower temperature limit
    mode = pEnc->read();
    if (mode != 4) {
      int k = pIron->changePID(mode, -1);
      pEnc->reset(k, 0, 5000, 1, 5);
    } else {
      pEnc->reset(temp_set, 0, 970, 1, 5);
    }
  } else {                                      // upper or lower temperature limit just setup
    mode = 0;
    pEnc->reset(1, 1, 4, 1, 1, true);           // 1 - Kp, 2 - Ki, 3 - Kd, 4 - temp
  }
  return this;
}

SCREEN* pidSCREEN::menu_long(void) {
  bool on = pIron->isOn();
  pIron->switchPower(!on);
  if (on)
    Serial.println("The iron is OFF");
  else
    Serial.println("The iron is ON");
  return this;
}
