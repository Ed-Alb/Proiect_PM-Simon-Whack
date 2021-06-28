#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "pitches.h"
#include "songs.h"
#include "ledPlay.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

/* PIN PWM SERVOMOTOR */
#define SERVOPIN 3

#define LEDSNO         5
#define INITIAL_DELAY  1300
#define DEFAULT_DELAY  1750
#define SUBTRACT_DELAY 350
#define SERVOSTARTPOS  120
#define SERVONOTICK    120

#define FIRSTLED       180
#define SECONDLED      135
#define THIRDLED       80
#define FOURTHLED      30
#define LASTLED        1

#define WAL            "WaL"
#define SIMSAYS        "SimSays"
#define EMPTYSTR       ""

#define LCD_FIRSTLINE  0
#define LCD_SECONDLINE 1

/* Player idexes */
#define P1 0
#define P2 1

/* Analog Pin 0 */
#define VRx A0
/* Digital Pin 4 */
#define SW 4

#define RIGHT_TRIANGLE 1
#define LEFT_TRIANGLE  0
#define HAPPYFACE      3
#define SADFACE        2
#define DUCK           4


/* Se incepe de la led-ul din mijloc */
uint8_t g_whichLed = 2;

/* 
 *  Variabila este folosita pentru a putea scrie corect pe ecran
 *  atunci când se schimbă numărul de cifre ale unui număr
*/
uint8_t g_digits = 1;

/* Ce Led trebuie înfrânt */
uint8_t g_whackLed = -1;

/*
 *  Variabile folositoare pentru a avansa în cadrul jocurilor
*/
int g_level    = 1;
int g_rounds   = 1;
int g_ledLives = 0;


volatile int countFiftyA = 0;

/* Index ce specifica randul carui jucator este */
int g_turn = 0;
int g_idx = 0;

int g_initialDelay = INITIAL_DELAY;

int g_lives[2] = {3, 3};
int g_randomArr[Sequence_Len];
int g_scoruriJucatori[2] = {0, 0};
int g_ledSounds[5] = {NOTE_A4, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_E5};
int g_servoPositions[5] = {FIRSTLED, SECONDLED, THIRDLED, FOURTHLED, LASTLED};

bool g_select         = false;
bool g_gameSelect     = true;
bool g_wrongSelection = false;
bool g_moveServoBack  = false;

String g_game  = EMPTYSTR;
String g_score = EMPTYSTR;
String g_Whack = String(WAL);
String g_Simon = String(SIMSAYS);

/*
   Întreruperea pentru butonul Joystick-ului
*/
ISR(PCINT2_vect)
{
  /* Debounce */
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (((PIND & (1 << PD4)) == 0 )&& (interrupt_time - last_interrupt_time > 525))
  {
    /* Apăsarea butonului de pe Joystick pentru jocul Simon Says */
    if ((g_select) && (g_game == g_Simon))
    {
      if (g_randomArr[g_idx] == g_whichLed)
      {
        tone(BUZZPIN, g_ledSounds[g_whichLed], 300);
        g_scoruriJucatori[g_turn] += 1;
        g_idx++;
      }
      else
      {
        g_lives[g_turn]--;
        g_select = false;
        g_wrongSelection = true;
        g_idx = 0;
      }
    }

    /* Apăsarea butonului de pe Joystick pentru jocul Whack-a-Led */
    if ((g_select) && (g_game == g_Whack))
    {
      if (g_whackLed == g_whichLed)
      {
        g_ledLives--;
        tone(BUZZPIN, g_ledSounds[g_whichLed], 300);
        g_scoruriJucatori[P1] += 1;
      }
      else
      {
        g_lives[P1]--;
        g_select = false;
        g_wrongSelection = true;
      }
    }
  }
  last_interrupt_time = interrupt_time;
}

/*
 *    Întreruperea ce face apinderea si stingerea
 *  LED-ul selectat cu joystick-ul la fiecare secundă
*/
int currentLed = 5;
ISR(TIMER1_COMPA_vect)
{
  if ((currentLed != g_whichLed) && (g_game != g_Whack))
  {
    digitalWrite(g_LEDS[currentLed], LOW);
    currentLed = g_whichLed;
    digitalWrite(g_LEDS[currentLed], HIGH);
    countFiftyA = 0;
  }
  else if ((g_select && countFiftyA > 50) && (g_game != g_Whack))
  {
    currentLed = g_whichLed;
    digitalWrite(g_LEDS[currentLed], !digitalRead(g_LEDS[currentLed]));

    for (int i = 0; i < LEDSNO; i++)
    {
      if (i != currentLed)
      {
        digitalWrite(g_LEDS[i], LOW);
      }
    }
    countFiftyA = 0;
  }

  if ((g_gameSelect) && (countFiftyA > 50))
  {
    int x = random(0, 5);
    digitalWrite(g_LEDS[x], !digitalRead(g_LEDS[x]));
    countFiftyA = 0;
  }

  if (g_moveServoBack)
  {
    g_moveServoBack = false;
  }
  
  countFiftyA++;
  digitalWrite(SERVOPIN, HIGH);
}

ISR(TIMER1_COMPB_vect)
{
  digitalWrite(SERVOPIN, LOW);
}

/* Enable timer compare interrupt */
void init_timer1(void)
{
  /* pe ocr1a fac blink led-urile */
  TIMSK1 |= (1 << OCIE1A);
  /* Pe ocr1b se mișcă servomotorul */
  TIMSK1 |= (1 << OCIE1B);
}

void configure_timer1(void)
{
  /*
      Configurare Timer 1 în mod CTC care va genera
      întreruperi cu frecvența de 50Hz
  */
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  /* numar 20 ms (16M / 64 / 50Hz - 1) */
  OCR1A = 4999;
  /* 0 grade */
  OCR1B = SERVO_convert_from_degrees(0);
  /* Mod CTC */
  TCCR1B |= (1 << WGM12);
  /* Prescaler 64 */
  TCCR1B |= (1 << CS11) | (1 << CS10);
}

uint8_t g_duck[8]         = {0x0, 0xc, 0x1d, 0xf, 0xf, 0x6, 0x0};
uint8_t g_leftTrangle[8]  = {0x0, 0x0, ~0x1b, ~0x13, ~0x3, ~0x13, ~0x1b, 0x0};
uint8_t g_rightTrangle[8] = {0x0, 0x0, ~0x1b, ~0x19, ~0x18, ~0x19, ~0x1b, 0x0};
uint8_t g_sadFace[8]      = {~0x11, ~0xe, ~0x4, ~0xe, ~0xa, ~0x4, ~0xe, ~0x11};
uint8_t g_happyFace[8]    = {~0x11, ~0xe, ~0x4, ~0xe, ~0x4, ~0xa, ~0xe, ~0x11};
/* Initializarile necesare: */
void setup()
{
  lcd.begin();
  lcd.backlight();

  lcd.createChar(0, g_leftTrangle);
  lcd.createChar(1, g_rightTrangle);
  lcd.createChar(2, g_sadFace);
  lcd.createChar(3, g_happyFace);
  lcd.createChar(4, g_duck);
  lcd.home();

  lcd.print("Choose the Game:");
  lcd.setCursor(0, 1);
  lcd.print(g_Simon);
  lcd.setCursor(g_Simon.length() + 2, 1);
  lcd.write(LEFT_TRIANGLE);
  lcd.setCursor(g_Simon.length() + 3, 1);
  lcd.write(RIGHT_TRIANGLE);
  lcd.setCursor(g_Simon.length() + 6, 1);
  lcd.print(g_Whack);

  randomSeed(analogRead(A3));

  Serial.begin(9600);
  pinMode(SERVOPIN, OUTPUT);

  pinMode(LED7, OUTPUT);
  pinMode(LED8, OUTPUT);
  pinMode(LED9, OUTPUT);
  pinMode(LED10, OUTPUT);
  pinMode(LED11, OUTPUT);
  pinMode(BUZZPIN, OUTPUT);

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), blink, LOW);

  /* Pin-ul la care este conectat joystick-ul */
  pinMode(SW, INPUT_PULLUP);

  PCICR  |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT20);

  cli();
  configure_timer1();
  init_timer1();
  sei();
}

void ADC_init(void)
{
  ADMUX = (1 << REFS0);
  ADCSRA = (1 << ADEN) | (1 << ADLAR) | (7 << ADPS0);
}

void ADC_sc(void)
{
  ADCSRA |= (1 << ADSC);
}

int ADC_get(void)
{
  return ADCH;  
}

/* Se șterge de pe ecran o întreagă linie */
void LCD_clear_line(int line)
{
  lcd.setCursor(0, line);
  for (int n = 0; n < 16; n++)
  {
    lcd.print(" ");
  }
}

/*
     Functie ce actualizează scorul afișat pe ecran
   Este gandită ca atunci când se trece la un nr.
   mai mare sau mai mic de cifre față de numărul
   anterior, să golească întreaga linie.
*/
void afisare_scor(void)
{
  g_score = String(g_scoruriJucatori[g_turn]);
  if ((g_score.length() != g_digits) && (g_score != ""))
  {
    g_digits = g_score.length();
    LCD_clear_line(0);
  }

  lcd.setCursor(0, 0);
  lcd.print("Your Score: " + String(g_scoruriJucatori[g_turn]));
}

void LCD_print_prompt(String Top, String Bottom)
{
  lcd.print(Top);
  lcd.setCursor(0, 1);
  lcd.print(Bottom);
  delay(DEFAULT_DELAY);
}

bool g_canPress = false;
bool g_pressed = false;
bool playSongF = false;
void blink()
{
  if (g_gameSelect)
  {
    playSongF = true;
  }
  if (g_canPress == true)
  {
    g_pressed = true;
    g_canPress = false;
  }
}

void reset_arr(void)
{
  for (int i = 0; i < Sequence_Len; i++)
  {
    g_randomArr[i] = -1;
  }
}

bool wait_for_sequence(int whichPlayer)
{
  /* Partea de selectie a led-ului dorit din secvență. */
  g_whichLed = 2;
  currentLed = 2;
  g_idx = 0;
  g_select = true;
  while (g_idx < g_level)
  {
    g_canPress = true;
    afisare_scor();

    /*
       Dacă s-a greșit secvența, atunci
       se reia tot procesul de la început.
    */
    if (g_wrongSelection)
    {
      g_wrongSelection = false;
      return false;
    }

    int joystickVal = ADC_get();
    if (joystickVal > 2)
    {
      g_whichLed = (g_whichLed + 1) % LEDSNO;
    }
    else if (joystickVal < 1)
    {
      if (g_whichLed == 0)
      {
        g_whichLed = 4;
      }
      else
      {
        g_whichLed = (g_whichLed - 1) % LEDSNO;
      }
    }

    if(g_pressed == true)
    {
      if (g_scoruriJucatori[g_turn] > 0)
        g_scoruriJucatori[g_turn]--;

      afisare_scor();
      
      g_moveServoBack = true;
      OCR1B = SERVO_convert_from_degrees(g_servoPositions[g_randomArr[g_idx]]);
      delay(SEC);

      OCR1B = SERVO_convert_from_degrees(0);
      g_pressed = false;
    }
    
    delay(300);
  }

  g_select = false;
  afisare_scor();
  return true;
}

String wait_for_choice(void)
{
  int line = LCD_SECONDLINE;
  int left_off = 2, right_off = 3, sndgame_off = 6;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Choose the Game:");
  lcd.setCursor(0, 1);
  lcd.print(g_Simon);
  lcd.setCursor(g_Simon.length() + left_off, line);
  lcd.write(LEFT_TRIANGLE);
  lcd.setCursor(g_Simon.length() + right_off, line);
  lcd.write(RIGHT_TRIANGLE);
  lcd.setCursor(g_Simon.length() + sndgame_off, line);
  lcd.print(g_Whack);

  while (true)
  {
    ADC_sc();

    int joystickVal = ADC_get();
    /* S-a ales WaL */
    if (joystickVal == 3)
    {
      g_gameSelect = false;
      turn_off_all_leds(LEDSNO);
      delay(SEC / 4);
      lcd.setCursor(g_Simon.length() + 2, 1);
      lcd.write(SADFACE);
      lcd.setCursor(g_Simon.length() + 3, 1);
      lcd.write(HAPPYFACE);
      play_start_song(150);
      return g_Whack;
    }
    /* S-a ales Simon Says */
    else if (joystickVal == 0)
    {
      g_gameSelect = false;
      turn_off_all_leds(LEDSNO);
      delay(SEC / 4);
      lcd.setCursor(g_Simon.length() + 2, 1);
      lcd.write(HAPPYFACE);
      lcd.setCursor(g_Simon.length() + 3, 1);
      lcd.write(SADFACE);
      play_start_song(150);
      return g_Simon;
    }
    if (playSongF)
    {
      playSong();
      playSongF = false;
    }
  }
  return EMPTYSTR;
}

void showWinner(void)
{
  if (g_scoruriJucatori[0] > g_scoruriJucatori[1])
  {
    LCD_print_prompt("The Winner is: ", "Player " + String(P1 + 1));
  }
  else if (g_scoruriJucatori[0] < g_scoruriJucatori[1])
  {
    LCD_print_prompt("The Winner is: ", "Player " + String(P2 + 1));
  }
  else
  {
    lcd.print("Egalitate!!");
  }
}

/* Funcție de resetare a valorilor, pentru un nou joc */
void New_Game(int whichPlayer)
{
  if (whichPlayer == P2)
  {
    g_turn = 0;
    g_scoruriJucatori[0] = 0;
    g_scoruriJucatori[1] = 0;
    g_rounds = 1;
    g_level = 1;
    g_initialDelay = INITIAL_DELAY;
    lcd.clear();
  }
  else if (whichPlayer == P1)
  {
    LCD_print_prompt("Game over for", "Player: " + String(g_turn + 1));
    lcd.clear();
    g_turn++;
    LCD_print_prompt("Game Starts!", "Player: " + String(g_turn + 1));
    play_start_song(150);
    turn_on_all_leds(2, true, true, 200, LEDSNO);
    g_rounds = 1;
    g_level = 1;
    g_initialDelay = INITIAL_DELAY;
    lcd.clear();
  }
}

bool wait_for_whacking()
{
  g_whichLed = 0;
  g_select = true;
  int changeSpeed = random(150, 300);
  g_ledLives = random(2, 6);
  while(true)
  {
    g_canPress = true;
    afisare_scor();
    lcd.setCursor(0, 1);
    lcd.print("Lives: " + String(g_lives[g_turn]));
    lcd.setCursor(8, 1);
    lcd.write(DUCK);
    lcd.setCursor(9, 1);
    lcd.print("BOSS: " + String(g_ledLives));
    digitalWrite(g_LEDS[g_whichLed], HIGH);
    delay(changeSpeed);
    digitalWrite(g_LEDS[g_whichLed], LOW);
    if (g_ledLives == 0)
    {
      afisare_scor();
      lcd.setCursor(0, 1);
      lcd.print("Lives: " + String(g_lives[g_turn]));
      lcd.setCursor(8, 1);
      lcd.write(HAPPYFACE);
      lcd.setCursor(9, 1);
      lcd.print("BOSS: " + String(g_ledLives));
      break;
    }
    else if (g_select == false)
    {
      afisare_scor();
      lcd.setCursor(0, 1);
      lcd.print("Lives: " + String(g_lives[g_turn]));
      lcd.setCursor(8, 1);
      lcd.write(SADFACE);
      lcd.setCursor(9, 1);
      lcd.print("BOSS: " + String(g_ledLives));
      break;
    }

    if(g_pressed == true)
    {
      if (g_scoruriJucatori[g_turn] > 0)
      {
        g_scoruriJucatori[g_turn]--;
      }

      afisare_scor();
      
      g_moveServoBack = true;
      OCR1B = SERVO_convert_from_degrees(g_servoPositions[g_whackLed]);
      delay(SEC);

      OCR1B = SERVO_convert_from_degrees(0);
      g_pressed = false;
    }
    g_whichLed = (g_whichLed + 1) % LEDSNO;
  }

  if (g_wrongSelection)
  {
    g_wrongSelection = false;
    return false;
  }
  return true;
}

/* valoare intre 250-500, dar eu iau 200 minimul */
int SERVO_convert_from_degrees(int grade)
{
  if (grade)
  {
    int low = 1;
    int high = 180;
    int Servo_max = 500;
    int Servo_min = 200;
    double factor = ((Servo_max - Servo_min) / ((double) (high - low)));
    return (int) (Servo_min + factor * (grade - low));
  }
  /* Stai pe loc */
  return SERVONOTICK;
}

/* 
 *  Aici se începe jocul și se reia runda
*/
void loop()
{
  ADC_init();
 
  if (g_gameSelect)
  {
    g_game = wait_for_choice();
    lcd.clear();
    if (g_game == g_Simon)
    {
      LCD_print_prompt("Game Starts!", "Player: " + String(g_turn + 1));
      turn_on_all_leds(1, true, true, 150, LEDSNO);
      turn_on_all_leds(1, true, false, 150, LEDSNO);
      lcd.clear();
    }

    if (g_game == g_Whack)
    {
      LCD_print_prompt("Whack-a-Led", "Game Starts!");
      turn_on_all_leds(1, true, true, 150, LEDSNO);
      turn_on_all_leds(1, true, false, 150, LEDSNO);
      lcd.clear();
    }
  }

  if (g_game == g_Simon)
  {
    reset_arr();
    g_select = false;
    afisare_scor();
    lcd.setCursor(0, 1);
    lcd.print("Lives: " + String(g_lives[g_turn]));

    /* Afișarea secvenței random de culori si sunete */
    for (int i = 0; i < g_level; i++)
    {
      g_randomArr[i] = random(0, 5);
      digitalWrite(g_LEDS[g_randomArr[i]], HIGH);
      tone(BUZZPIN, g_ledSounds[g_randomArr[i]], 300);
      delay(g_initialDelay);
      digitalWrite(g_LEDS[g_randomArr[i]], LOW);
      delay(g_initialDelay);
    }
    turn_on_all_leds(1, false, true, 650, LEDSNO);

    bool result = wait_for_sequence(g_turn);
    if (result == false)
    {
      /*
         Dacă a greșit secvența, atunci runda
         nu va avansa ("se stă pe loc")
      */
      play_fail_song();
      turn_on_all_leds(3, false, true, 400, LEDSNO);
    }
    else
    {
      /* Trece la următoarea runda / următorul nivel */
      if (g_rounds % 3 == 0)
      {
        /* Următorul nivel */
        g_level++;
        g_initialDelay = INITIAL_DELAY;
        delay(HALFSEC);
        play_next_lvl_song();
        turn_on_all_leds(1, true, true, 200, LEDSNO);
        g_rounds++;
      }
      else
      {
        /* Următoarea rundă */
        delay(HALFSEC);
        play_right_song();
        turn_on_all_leds(1, false, true, 500, LEDSNO);
        g_initialDelay -= SUBTRACT_DELAY;
        g_rounds++;
      }
    }

    lcd.clear();
    if ((g_lives[P2] == 0) && (g_lives[P1] == 0))
    {
      OCR1B = SERVO_convert_from_degrees(0);
      g_lives[0] = 3;
      g_lives[1] = 3;
      g_gameSelect = true;
      LCD_print_prompt("Game over for", "Player: " + String(g_turn + 1));
      lcd.clear();

      LCD_print_prompt("Player 1: " + String(g_scoruriJucatori[0]), "Player 2: " + String(g_scoruriJucatori[1]));
      delay(3 * SEC);
  
      lcd.clear();

      /* Afișarea pe ecran a câștigătorului */
      showWinner();
  
      delay(3 * SEC);
  
      New_Game(P2);
    }
    else if ((g_lives[P1] == 0) && (g_turn == P1))
    {
      New_Game(P1);
    }
  }
  else if (g_game == g_Whack)
  {
    for (int i = 0; i < LEDSNO; i++)
    {
      digitalWrite(g_LEDS[i], LOW);
    }
    afisare_scor();
    lcd.setCursor(0, 1);
    lcd.print("Lives: " + String(g_lives[g_turn]));
    g_whackLed = random(0,5);
    digitalWrite(g_LEDS[g_whackLed], HIGH);
    tone(BUZZPIN, g_ledSounds[g_whackLed], 300);
    delay(g_initialDelay);
    digitalWrite(g_LEDS[g_whackLed], LOW);
    delay(g_initialDelay);

    /* 
     *  Stau în buclă până când se termină runda
     *  adică până când se greșește sau se alege
     *  alege corect led-ul bun
    */
    bool result = wait_for_whacking();

    if (result == false)
    {
      play_fail_song();
      turn_on_all_leds(3, false, true, 400, LEDSNO);
    }
    else
    {
      delay(HALFSEC);
      play_right_song();
      turn_on_all_leds(1, false, true, 500, LEDSNO);
    }
    
    if (g_lives[P1] == 0)
    {
      lcd.clear();
      LCD_print_prompt("Game Over!", "Final Score: " + String(g_scoruriJucatori[P1]));
      lcd.clear();
      g_lives[P1] = 3;
      g_scoruriJucatori[P1] = 0;
      g_gameSelect = true;
      g_game = "";
    }
  }
}

#define REST 0
// change this to make the song slower or faster
int tempo = 114;

int16_t melody[] = {
  NOTE_D5,-4, NOTE_E5,-4, NOTE_A4,4, //1
  NOTE_E5,-4, NOTE_FS5,-4, NOTE_A5,16, NOTE_G5,16, NOTE_FS5,8,
  NOTE_D5,-4, NOTE_E5,-4, NOTE_A4,2,
  NOTE_A4,16, NOTE_A4,16, NOTE_B4,16, NOTE_D5,8, NOTE_D5,16,
  NOTE_D5,-4, NOTE_E5,-4, NOTE_A4,4, //repeat from 1
  NOTE_E5,-4, NOTE_FS5,-4, NOTE_A5,16, NOTE_G5,16, NOTE_FS5,8,
  NOTE_D5,-4, NOTE_E5,-4, NOTE_A4,2,
  NOTE_A4,16, NOTE_A4,16, NOTE_B4,16, NOTE_D5,8, NOTE_D5,16,
  REST,4, NOTE_B4,8, NOTE_CS5,8, NOTE_D5,8, NOTE_D5,8, NOTE_E5,8, NOTE_CS5,-8,
  NOTE_B4,16, NOTE_A4,2, REST,4
};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;

void playSong(void)
{
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(BUZZPIN, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(BUZZPIN);
  }  
}
