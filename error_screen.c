//---------------------------------------- class errorSCREEN [the soldering iron error detected] ---------------
class errorSCREEN : public SCREEN {
  public:
    errorSCREEN(DSPL* DSP, BUZZER* BZ) {
      pD  = DSP;
      pBz = BZ;
    }
    virtual void init(void) { pD->clear(); pD->msgFail(); pBz->failedBeep(); }
  private:
    DSPL*   pD;                                 // Pointer to the display instance
    BUZZER* pBz;                                // Pointer to the buzzer instance
};
