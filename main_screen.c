//---------------------------------------- class mainSCREEN [the soldering iron is OFF] ------------------------
class mainSCREEN : public SCREEN {
  public:
    mainSCREEN(IRON* Iron, DSPL* DSP, ENCODER* ENC, BUZZER* Buzz, CONFIG* Cfg) {
      update_screen = 0;
      pIron = Iron;
      pD = DSP;
      pEnc = ENC;
      pBz = Buzz;
      pCfg = Cfg;
      is_celsius = true;
    }
    virtual void init(void);
    virtual void show(void);
    virtual void rotaryValue(int16_t value);
  private:
    IRON*    pIron;                             // Pointer to the iron instance
    DSPL*    pD;                                // Pointer to the DSPLay instance
    ENCODER* pEnc;                              // Pointer to the rotary encoder instance
    BUZZER*  pBz;                               // Pointer to the simple buzzer instance
    CONFIG*  pCfg;                              // Pointer to the configuration instance
    uint32_t update_screen;                     // Time in ms to switch information on the display
    bool     used;                              // Whether the iron was used (was hot)
    bool     cool_notified;                     // Whether there was cold notification played
    bool     is_celsius;                        // The temperature units (Celsius or Farenheit)
	  const uint16_t period = 1000;               // The period to update the screen
};

void mainSCREEN::init(void) {
  pIron->switchPower(false);
  uint16_t temp_set = pIron->getTemp();
  is_celsius = pCfg->getTempUnits();
  pIron->setTempUnits(is_celsius);
  uint16_t tempH = pIron->temp2humanUnits(temp_set);
  if (is_celsius)
    pEnc->reset(tempH, temp_minC, temp_maxC, 1, 5);
  else
    pEnc->reset(tempH, temp_minF, temp_maxF, 1, 5);
  update_screen = 0;
  pD->clear();
  used = pIron->used();
  cool_notified = !used;
  if (used) {                                   // the iron was used, we should save new data in EEPROM
    pCfg->saveTemp(temp_set);
  }
}

void mainSCREEN::rotaryValue(int16_t value) {
  update_screen = millis() + period;
  pIron->setTempHumanUnits(value);
  pD->tSet(value, is_celsius);
}

void mainSCREEN::show(void) {
  if ((!force_redraw) && (millis() < update_screen)) return;

  force_redraw = false;
  update_screen = millis() + period;

  uint16_t temp_set = pIron->getTemp();
  temp_set = pIron->temp2humanUnits(temp_set);
  pD->tSet(temp_set, is_celsius);
  pD->msgOff();

  if (pIron->noIron()) {                        // No iron connected
    pD->msgNoIron();
    return;
  }

  uint16_t temp = pIron->tempAverage();
  temp = pIron->temp2humanUnits(temp);
  if (pIron->isCold()) {
    if (used)
      pD->msgCold();
    else
      pD->tCurr(temp);
    if (!cool_notified) {
      pBz->lowBeep();
      cool_notified = true;
    }
  } else {
    pD->tCurr(temp);
  }
}
