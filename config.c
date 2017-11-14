
//------------------------------------------ Configuration data ------------------------------------------------
/* Config record in the EEPROM has the following format:
  uint32_t ID                           each time increment by 1
  struct cfg                            config data, 8 bytes
  byte CRC                              the checksum
*/
struct cfg {
  uint16_t temp_min;                            // The minimum temperature (180 centegrees)
  uint16_t temp_max;                            // The temperature for 400 centegrees
  uint16_t temp;                                // The temperature of the iron to be start
  byte     off_timeout;                         // The Automatic switch-off timeout in minutes [0 - 30]
  bool     celsius;                             // Temperature units: true - celsius, false - farenheit
};

class CONFIG {
  public:
    CONFIG() {
      can_write = is_valid = false;
      buffRecords = 0;
      rAddr = wAddr = 0;
      eLength = 0;
      nextRecID = 0;
    }
    void init();
    bool load(void);
    bool isValid(void)       { return is_valid; }
    uint16_t temp(void)      { return Config.temp; }
    byte getOffTimeout(void) { return Config.off_timeout; }
    bool getTempUnits(void)  { return Config.celsius; }
    bool saveTemp(uint16_t t);
    void saveConfig(byte off, bool cels);
    void saveCalibrationData(uint16_t t_max, uint16_t t_min);
    void getCalibrationData(uint16_t& t_max, uint16_t& t_min);
    void setDefaults(bool Write = false);
  private:
    struct cfg Config;
    bool readRecord(uint16_t addr, uint32_t &recID);
    bool save(void);
    bool can_write;                             // Tha flag indicates that data can be saved
    bool is_valid;                              // Whether tha data was loaded
    byte buffRecords;                           // Number of the records in the outpt buffer
    uint16_t rAddr;                             // Address of thecorrect record in EEPROM to be read
    uint16_t wAddr;                             // Address in the EEPROM to start write new record
    uint16_t eLength;                           // Length of the EEPROM, depends on arduino model
    uint32_t nextRecID;                         // next record ID
    const byte record_size = 16;                // The size of one record in bytes
    const uint16_t def_min = 554;               // Default minimum temperature
    const uint16_t def_max = 916;               // Default maximun temperature
    const uint16_t def_set = 629;               // Default setup temperature
};

 // Read the records until the last one, point wAddr (write address) after the last record
void CONFIG::init(void) {
  eLength = EEPROM.length();
  byte t, p ,h;
  uint32_t recID;
  uint32_t minRecID = 0xffffffff;
  uint16_t minRecAddr = 0;
  uint32_t maxRecID = 0;
  uint16_t maxRecAddr = 0;
  byte records = 0;

  setDefaults();
  nextRecID = 0;

  // read all the records in the EEPROM find min and max record ID
  for (uint16_t addr = 0; addr < eLength; addr += record_size) {
    if (readRecord(addr, recID)) {
      ++records;
      if (minRecID > recID) {
        minRecID = recID;
        minRecAddr = addr;
      }
      if (maxRecID < recID) {
        maxRecID = recID;
        maxRecAddr = addr;
      }
    } else {
      break;
    }
  }

  if (records == 0) {
    wAddr = rAddr = 0;
    can_write = true;
    return;
  }

  rAddr = maxRecAddr;
  if (records < (eLength / record_size)) {      // The EEPROM is not full
    wAddr = rAddr + record_size;
    if (wAddr > eLength) wAddr = 0;
  } else {
    wAddr = minRecAddr;
  }
  can_write = true;
}

bool CONFIG::saveTemp(uint16_t t) {
  if (t == Config.temp) return true;
  Config.temp = t;
  return save();
}

void CONFIG::saveConfig(byte off, bool cels) {
  if (off > 30) off = 0;
  Config.off_timeout = off;
  Config.celsius = cels;
  save();                                       // Save new data into the EEPROM
}

void CONFIG::saveCalibrationData(uint16_t t_max, uint16_t t_min) {
  Config.temp_max  = t_max;
  Config.temp_min  = t_min;
  save();
}

void CONFIG::getCalibrationData(uint16_t& t_max, uint16_t& t_min) {
  t_max = Config.temp_max;
  t_min = Config.temp_min;
}

bool CONFIG::save(void) {
  if (!can_write) return can_write;
  if (nextRecID == 0) nextRecID = 1;

  uint16_t startWrite = wAddr;
  uint32_t nxt = nextRecID;
  byte summ = 0;
  for (byte i = 0; i < 4; ++i) {
    EEPROM.write(startWrite++, nxt & 0xff);
    summ <<=2; summ += nxt;
    nxt >>= 8;
  }
  byte* p = (byte *)&Config;
  for (byte i = 0; i < sizeof(struct cfg); ++i) {
    summ <<= 2; summ += p[i];
    EEPROM.write(startWrite++, p[i]);
  }
  summ ++;                                      // To avoid empty records
  EEPROM.write(wAddr+record_size-1, summ);

  rAddr = wAddr;
  wAddr += record_size;
  if (wAddr > EEPROM.length()) wAddr = 0;
  return true;
}

bool CONFIG::load(void) {

  is_valid = readRecord(rAddr, nextRecID);
  nextRecID ++;
  if (is_valid) {
    if (Config.temp_min >= Config.temp_max) {
      setDefaults();
    }
    if ((Config.temp > Config.temp_max) || (Config.temp < Config.temp_min)) Config.temp = def_set;
    if ((Config.off_timeout > 30) ) Config.off_timeout = 0;
  }
  return is_valid;
}

bool CONFIG::readRecord(uint16_t addr, uint32_t &recID) {
  byte Buff[16];

  for (byte i = 0; i < 16; ++i)
    Buff[i] = EEPROM.read(addr+i);

  byte summ = 0;
  for (byte i = 0; i < sizeof(struct cfg) + 4; ++i) {

    summ <<= 2; summ += Buff[i];
  }
  summ ++;                                      // To avoid empty fields
  if (summ == Buff[15]) {                       // Checksumm is correct
    uint32_t ts = 0;
    for (char i = 3; i >= 0; --i) {
      ts <<= 8;
      ts |= Buff[i];
    }
    recID = ts;
    byte i = 4;
    memcpy(&Config, &Buff[4], sizeof(struct cfg));
    return true;
  }
  return false;
}

void CONFIG::setDefaults(bool Write) {          // Restore default values
  Config.temp        = def_set;
  Config.temp_min    = def_min;
  Config.temp_max    = def_max;
  Config.off_timeout = 0;                       // Default autometic switch-off timeout (disabled)
  Config.celsius     = true;                    // Default use celsius
  if (Write) {
    save();
  }
}
