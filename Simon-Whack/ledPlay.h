/* pinii LED-urilor */
#define LED7  7
#define LED8  8
#define LED9  9
#define LED10 10
#define LED11 11

#define SEC     1000
#define HALFSEC 500

#define Sequence_Len 40

uint8_t g_LEDS[5] = {LED7, LED8, LED9, LED10, LED11};

void turn_on_all_leds(int rounds, bool sequence, bool leftRight, int myDelay, int LEDSNO)
{
  for (int i = 0; i < rounds; i++)
  {
    if (leftRight)
    {
      for (int j = 0; j < LEDSNO; j++)
      {
        digitalWrite(g_LEDS[j], HIGH);
        if (sequence)
        {
          delay(myDelay);
        }
      }
      if (!sequence)
      {
        delay(myDelay);
      }
      for (int j = 0; j < LEDSNO; j++)
      {
        digitalWrite(g_LEDS[j], LOW);
        if (sequence)
          delay(myDelay);
      }
      if (!sequence)
      {
        delay(myDelay);
      }
    }
    else
    {
      for (int j = LEDSNO; j >= 0; j--)
      {
        digitalWrite(g_LEDS[j], HIGH);
        if (sequence)
        {
          delay(myDelay);
        }
      }
      if (!sequence)
      {
        delay(myDelay);
      }
      for (int j = LEDSNO; j >= 0; j--)
      {
        digitalWrite(g_LEDS[j], LOW);
        if (sequence)
        {
          delay(myDelay);
        }
      }
      if (!sequence)
      {
        delay(myDelay);
      }
    }
  }
}

void turn_off_all_leds(int LEDSNO)
{
  for (int j = 0; j < LEDSNO; j++)
  {
      digitalWrite(g_LEDS[j], LOW);
  }  
}
