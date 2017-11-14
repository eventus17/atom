
//------------------------------------------ class HISTORY ----------------------------------------------------
#define H_LENGTH 16
class HISTORY {
  public:
    HISTORY(void)                               { len = 0; }
    void     init(void)                         { len = 0; }
    bool     isFull(void)                       { return len == H_LENGTH; }
    uint16_t last(void);
    uint16_t top(void)                          { return queue[0]; }
    void     put(uint16_t item);                // Put new entry to the history
    uint16_t average(void);                     // calcilate the average value
    float    dispersion(void);                  // calculate the math dispersion
    float    gradient(void);                    // calculate the gradient of the history values
  private:
    uint16_t queue[H_LENGTH];
    byte len;                                   // The number of elements in the queue
    byte index;                                 // The current element position, use ring buffer
};

void HISTORY::put(uint16_t item) {
  if (len < H_LENGTH) {
    queue[len++] = item;
  } else {
    queue[index ] = item;
    if (++index >= H_LENGTH) index = 0;         // Use ring buffer
  }
}

uint16_t HISTORY::last(void) {
  byte i = H_LENGTH - 1;
  if (index)
    i = index - 1;
  return queue[i];
}

uint16_t HISTORY::average(void) {
  uint32_t sum = 0;
  if (len == 0) return 0;
  if (len == 1) return queue[0];
  for (byte i = 0; i < len; ++i) sum += queue[i];
  sum += len >> 1;                              // round the average
  sum /= len;
  return uint16_t(sum);
}

float HISTORY::dispersion(void) {
  if (len < 3) return 1000;
  uint32_t sum = 0;
  uint32_t avg = average();
  for (byte i = 0; i < len; ++i) {
    long q = queue[i];
    q -= avg;
    q *= q;
    sum += q;
  }
  sum += len << 1;
  float d = (float)sum / (float)len;
  return d;
}

// approfimating the history with the line (y = ax+b) using method of minimum square. Gradient is parameter a
float HISTORY::gradient(void) {
  if (len < 2) return 0;
  long sx, sx_sq, sxy, sy;
  sx = sx_sq = sxy = sy = 0;
  for (byte i = 1; i <= len; ++i) {
    sx    += i;
    sx_sq += i*i;
    sxy   += i*queue[i-1];
    sy    += queue[i-1];
  }
  long numerator   = len * sxy - sx * sy;
  long denominator = len * sx_sq - sx * sx;
  float a = (float)numerator / (float)denominator;
  return a;
}
