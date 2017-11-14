//---------------------------------------- class workSCREEN [the soldering iron is ON] -------------------------
class workSCREEN : public SCREEN {
  public:
    workSCREEN(IRON* Iron, DSPL* DSP, ENCODER* Enc, BUZZER* Buzz, CONFIG* Cfg) {
      update_screen = 0;
      pIron = Iron;
      pD    = DSP;
      pBz   = Buzz;
      pEnc  = Enc;
      pCfg  = Cfg;
      ready = false;
    }
    virtual void init(void);
    virtual void show(void);
    virtual void rotaryValue(int16_t value);
    virtual SCREEN* returnToMain(void);
  private:
    uint32_t update_screen;                     // Time in ms to update the screen
    IRON*    pIron;                             // Pointer to the iron instance
    DSPL*    pD;                                // Pointer to the DSPLay instance
    BUZZER*  pBz;                               // Pointer to the simple Buzzer instance
    ENCODER* pEnc;                              // Pointer to the rotary encoder instance
    CONFIG*  pCfg;                              // Pointer to the configuration instance
    bool     ready;                             // Whether the iron is ready
    uint32_t  auto_off_notified;                // The time (in ms) when the automatic power-off was notified
    HISTORY  idle_power;                        // The power supplied to the iron when it is not used
	  const uint16_t period = 1000;               // The period to update the screen (ms)
};

void workSCREEN::init(void) {
  uint16_t temp_set = pIron->getTemp();
  bool is_celsius = pIron->getTempUnits();
  uint16_t tempH = pIron->temp2humanUnits(temp_set);
  if (is_celsius)
    pEnc->reset(tempH, temp_minC, temp_maxC, 1, 5);
  else
    pEnc->reset(tempH, temp_minF, temp_maxF, 1, 5);
  pIron->switchPower(true);
  ready = false;
  pD->clear();
  pD->tSet(tempH, is_celsius);
  pD->msgOn();
  forceRedraw();
  uint16_t to = pCfg->getOffTimeout() * 60;
  this->setSCRtimeout(to);
  idle_power.init();
  auto_off_notified = 0;
}

void workSCREEN::rotaryValue(int16_t value) {
  ready = false;
  pD->msgOn();
  update_screen = millis() + period;
  pIron->setTempHumanUnits(value);
  pD->tSet(value, pIron->getTempUnits());
  idle_power.init();
  SCREEN::resetTimeout();
}

void workSCREEN::show(void) {
  if ((!force_redraw) && (millis() < update_screen)) return;

  force_redraw = false;
  update_screen = millis() + period;

  int temp = pIron->tempAverage();
  int temp_set = pIron->getTemp();
  int tempH = pIron->temp2humanUnits(temp);
  pD->tCurr(tempH);
  byte p = pIron->appliedPower();
  pD->percent(p);

  uint16_t td = pIron->tempDispersion();
  uint16_t pd = pIron->powerDispersion();
  int ip      = idle_power.average();
  int ap      = pIron->getAvgPower();
  if ((temp <= temp_set) && (temp_set - temp <= 3) && (td <= 3) && (pd <= 4)) {
    idle_power.put(ap);
  }
  if (ap - ip >= 2) {    // The iron was used
    SCREEN::resetTimeout();
    if (ready) {
      idle_power.init();
      idle_power.put(ip+1);
    }
  }

  if ((abs(temp_set - temp) < 3) && (pIron->tempDispersion() <= 3))  {
    if (!ready) {
      idle_power.put(ap);
      pBz->shortBeep();
      pD->msgReady();
      ready = true;
    } else {
      if (ready && SCREEN::wasRecentlyReset()) {
        pD->msgWorking();
        auto_off_notified = 0;
      } else {
        uint32_t to = (time_to_return - millis()) / 1000;
        if ((scr_timeout > 0) && (to < 100)) {
          pD->timeToOff(to);
          if (!auto_off_notified || ((millis() - auto_off_notified) > 300000)) {
            pBz->shortBeep();
            auto_off_notified = millis();
          }
        } else
          pD->msgReady();
      }
    }
  }
}

SCREEN* workSCREEN::returnToMain(void) {
  if (main && (scr_timeout != 0) && (millis() >= time_to_return)) {
    scr_timeout = 0;
    pBz->doubleBeep();
    return main;
  }
  return this;
}
