#define BUZZPIN 12

void play_start_song(int theDelay)
{
  tone(BUZZPIN,NOTE_A5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_B5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_C5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_B5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_C5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_D5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_C5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_D5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_E5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_D5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_E5);
  delay(theDelay);
  tone(BUZZPIN,NOTE_E5);
  delay(theDelay);
  noTone(BUZZPIN);
}

void play_fail_song(void)
{
  tone(BUZZPIN,NOTE_GS1);
  delay(200);
  tone(BUZZPIN, NOTE_B2);
  delay(200);
  noTone(BUZZPIN);
}

void play_next_lvl_song(void) {
  tone(BUZZPIN,NOTE_FS5);
  delay(200);
  tone(BUZZPIN, NOTE_D5);
  delay(200);
  tone(BUZZPIN, NOTE_E6);
  delay(200);
  noTone(BUZZPIN);
}

void play_right_song(void)
{
  tone(BUZZPIN, NOTE_E6);
  delay(200);
  tone(BUZZPIN, NOTE_D5);
  delay(200);
  noTone(BUZZPIN);
}
