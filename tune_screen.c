//---------------------------------------- class tuneSCREEN [tune the register and calibrating the iron] -------
class tuneSCREEN : public SCREEN {
  public:
    tuneSCREEN(IRON* Iron, DSPL* DSP, ENCODER* ENC, BUZZER* Buzz, CONFIG* Cfg) {
      update_screen = 0;
      pIron = Iron;
      pD = DSP;
      pEnc = ENC;
      pBz  = Buzz;
      pCfg = Cfg;
    }
    virtual void init(void);
    virtual SCREEN* menu(void);
    virtual SCREEN* menu_long(void);
    virtual void show(void);
    virtual void rotaryValue(int16_t value);
  private:
    IRON* pIron;                                // Pointer to the iron instance
    DSPL* pD;                                   // Pointer to the display instance
    ENCODER* pEnc;                              // Pointer to the rotary encoder instance
    BUZZER* pBz;                                // Pointer to the simple Buzzer instance
    CONFIG* pCfg;                               // Pointer to the configuration class
    byte mode;                                  // Which temperature to tune [0-3]: select, up temp, low temp, defaults
    bool arm_beep;                              // Whether beep is armed
    byte max_power;                             // Maximum possible power to be applied
    uint32_t update_screen;                     // Time in ms to switch information on the display
    uint16_t tul[2];                            // upper & lower temp
    byte pul[2];                                // upper and lower power
};

void tuneSCREEN::init(void) {
  max_power = pIron->getMaxFixedPower();
  mode = 0;                                     // select the element from the list
  pul[0] = 75; pul[1] = 20;
  pEnc->reset(0, 0, 4, 1, 1, true);             // 0 - up temp, 1 - low temp, 2 - defaults, 3 - save, 4 - cancel
  update_screen = millis();
  arm_beep = false;
  tul[0] = tul[1] = 0;
  pD->clear();
  pD->msgTune();
  pD->tempLim(0, 0);
  forceRedraw();
}

void tuneSCREEN::rotaryValue(int16_t value) {
  if (mode == 0) {                              // No limit is selected, list the menu
    switch (value) {
      case 2:
        pD->msgDefault();
        break;
      case 3:
        pD->msgApply();
        break;
      case 4:
        pD->msgCancel();
        break;
      default:
       pD->tempLim(value, tul[value]);
       break;
    }
  } else {
    pIron->fixPower(value);
    force_redraw = true;
  }
  update_screen = millis() + 1000;
}

void tuneSCREEN::show(void) {
  if ((!force_redraw) && (millis() < update_screen)) return;

  force_redraw = false;
  update_screen = millis() + 1000;
  if (mode != 0) {                              // Selected upper or lower temperature
    int16_t temp = pIron->tempAverage();
    pD->tCurr(temp);
    byte power = pEnc->read();                  // applied power
    power = map(power, 0, max_power, 0, 100);
    pD->percent(power);
    if (mode == 1)
      pD->msgUpper();
    else
      pD->msgLower();
  }
  if (arm_beep && (pIron->tempDispersion() < 5)) {
    pBz->shortBeep();
    arm_beep = false;
  }
}

SCREEN* tuneSCREEN::menu(void) {                // The rotary button pressed
  if (mode == 0) {                              // select upper or lower temperature limit
    int val = pEnc->read();
    if (val == 2) {                             // load defaults
      pCfg->setDefaults(true);                  // Write default config to the EEPROM
      if (main) return main;                    // Return to the main screen
    }
    if (val == 3) {
      menu_long();                              // Save the configuration to the EEPROM
      if (next) return next;
      return this;
    }
    if (val == 4) {
      if (next) return next;                    // Return to the previous menu
      return this;
    }
    mode = val + 1;
    pD->clear();
    pD->msgTune();
    switch (mode) {
      case 1:                                   // upper temp
        pD->msgUpper();
        break;
      case 2:                                   // lower temp
        pD->msgLower();
        break;
      default:
        break;
    }
    pEnc->reset(pul[mode-1], 0, max_power, 1, 5);
    pIron->fixPower(pul[mode-1]);               // Switch on the soldering iron
    arm_beep = true;
  } else {                                      // upper or lower temperature limit just setup
    pul[mode-1] = pEnc->read();                 // The supplied power
    tul[mode-1] = pIron->tempAverage();
    pD->clear();
    pD->msgTune();
    pEnc->reset(mode-1, 0, 4, 1, 1, true);      // 0 - up temp, 1 - low temp, 2 - defaults, 3 - save, 4 - cancel
    mode = 0;
    pIron->fixPower(0);
  }
  force_redraw = true;
  return this;
}

SCREEN* tuneSCREEN::menu_long(void) {
  pIron->fixPower(0);                           // switch off the power
  bool all_data = true;
  for (byte i = 0; i < 2; ++i) {
    if (!tul[i]) all_data = false;
  }
  if (all_data) {                               // save calibration data. Config will be written to the EEPROM later on
    pCfg->saveCalibrationData(tul[0], tul[1]);
    pIron->init(tul[0], tul[1]);
  }
  if (next) return next;
  return this;
}
