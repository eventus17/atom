//------------------------------------------ class SCREEN ------------------------------------------------------
class SCREEN {
  public:
    SCREEN* next;                               // Pointer to the next screen
    SCREEN* nextL;                              // Pointer to the next Level screen, usually, setup
    SCREEN* main;                               // Pointer to the main screen
    SCREEN() {
      next = nextL = main = 0;
      force_redraw = true;
      scr_timeout = 0;
      time_to_return = 0;
    }
    virtual void    init(void)                  { }
    virtual void    show(void)                  { }
    virtual SCREEN* menu(void)                  {if (this->next != 0) return this->next; else return this; }
    virtual SCREEN* menu_long(void)             { if (this->nextL != 0) return this->nextL; else return this; }
    virtual void    rotaryValue(int16_t value)  { }
    virtual SCREEN* returnToMain(void);         // Return to the main screen in the menu tree
    bool            isSetup(void)               { return (scr_timeout != 0); }
    void            forceRedraw(void)           { force_redraw = true; }
    void            resetTimeout(void);         // Reset automatic return timeout
    void            setSCRtimeout(uint16_t t)   { scr_timeout = t; resetTimeout(); }
    bool            wasRecentlyReset(void);     // Whether the return timeout was reset in the last 15 seconds
  protected:
    bool force_redraw;
    uint16_t scr_timeout;                       // Timeout is sec. to return to the main screen, canceling all changes
    uint32_t time_to_return;                    // Time in ms to return to main screen
};

SCREEN* SCREEN::returnToMain(void) {
  if (main && (scr_timeout != 0) && (millis() >= time_to_return)) {
    scr_timeout = 0;
    return main;
  }
  return this;
}

void SCREEN::resetTimeout(void) {
  if (scr_timeout > 0)
    time_to_return = millis() + (uint32_t)scr_timeout*1000;
}

bool SCREEN::wasRecentlyReset(void) {
  uint32_t to = (time_to_return - millis()) / 1000;
  return((scr_timeout - to) < 15);
}
