/*
This mini game is a module of the video game "No Talking and nobody explodes"
The object of the game is to ANSroduce a sequence of LEDs by isPresseding the corresponding buttons in the defusing manual.
The game is made up of 4 buttons and 4 leds.
The JAUNE button launches the game.
Then the game generates a sequence of random LEDs.
The player must ANSroduce the sequence by isPresseding the corresponding buttons.
If the player succeeds in the sequence, he advances to the next level.
If the player fails, he must start the sequence again.
The game has 5 difficulty levels.
Level 1 is the easiest and level 5 is the most difficult.
The game is over when the player reaches level 5.
*/

// Add score for players

#include <Arduino.h>
#include "Bouton.h"
#include "rgb_lcd.h"
#include <WiFi.h>
#include <esp_now.h>

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
/*
uint8_t simonAddress[] = {0x10, 0x97, 0xBD, 0xD2, 0x9B, 0x54};
uint8_t labyrintheAddress[] = {0xC8, 0xF0, 0x9E, 0x2C, 0x12, 0xC8};
uint8_t chronoAddress[] = {0xC8, 0xF0, 0x9E, 0x2B, 0xF7, 0x44};
*/

typedef struct struct_message
{
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;
struct_message dataRecv;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&dataRecv, incomingData, sizeof(dataRecv));
  Serial.print("\r\nBytes received: ");
  Serial.println(len);
  Serial.print("From slave: ");
  Serial.println(dataRecv.a);
  Serial.println();
}

Bouton bt[4];
rgb_lcd lcd;

// Function prototype
void setup_bt(int nb_bt);
void read_bt(int nb_bt);

void setup_led(int nb_led);
bool sequence_led(int level);
void algo_led_random(void);
void algo_led_random_place(void);
void algo_answer(void);

void led_PWM(int led_pin, int pwm[3]);
void led_PWM_Off(int led_num);
void player_answer(void);
void error_Answer(int delai);
void rainbow_led(void);
void smooth_RGB(void);

void test_all_led(void);
void test_PWM(void);
void reinitialize_color(int bt);
void change_color(int bt, int R, int G, int B);

#define ROUGE 0
#define BLEU 1
#define VERT 2
#define JAUNE 3

#define PWMROUGE \
  {              \
    1020, 0, 0   \
  }
#define PWMBLEU \
  {             \
    0, 0, 1020  \
  }
#define PWMVERT \
  {             \
    0, 1020, 0  \
  }
#define PWMJAUNE  \
  {               \
    1020, 1020, 0 \
  }
#define PWMWHITE     \
  {                  \
    1020, 1020, 1020 \
  }
#define PWM_OFF \
  {             \
    0, 0, 0     \
  }

#define INIT 0
#define TEST 1
#define GAME 2

#define SEQ 0
#define ANS 1
#define ERR 2
#define WIN 3

int mini_game[4] = {0, 0, 0, 1};
// mini_game: 1st index: id_module, 2nd index: level_max, 3rd index: level, 4th index: mode_jeu
int server_state_system = 0;

// Let's define the pins of the buttons
const int pin_bt_BT1 = 34;
const int pin_bt_BT2 = 35;
const int pin_bt_BT3 = 32;
const int pin_bt_BT4 = 33;
u_int8_t button_pin[4] = {pin_bt_BT3, pin_bt_BT1, pin_bt_BT2, pin_bt_BT4};

// Let's define the pins of the leds
const int pin_led_BT1[3] = {23, 22, 21};
const int pin_led_BT2[3] = {19, 18, 5};
const int pin_led_BT3[3] = {17, 16, 4};
const int pin_led_BT4[3] = {2, 15, 0}; // Cour-circuit le pin sd1 vers GPIO 0

int led_pwm[4][3] = {PWMROUGE, PWMBLEU, PWMVERT, PWMJAUNE};
int led_color[4][3] = {PWMROUGE, PWMBLEU, PWMVERT, PWMJAUNE};

int led_pin[4][3] = {{pin_led_BT1[0], pin_led_BT1[1], pin_led_BT1[2]},
                     {pin_led_BT2[0], pin_led_BT2[1], pin_led_BT2[2]},
                     {pin_led_BT3[0], pin_led_BT3[1], pin_led_BT3[2]},
                     {pin_led_BT4[0], pin_led_BT4[1], pin_led_BT4[2]}};
u_int8_t led_place[4] = {ROUGE, BLEU, VERT, JAUNE};

// Led variables and buttons
unsigned long int led_time[4] = {0, 0, 0, 0};
int color_bt[4] = {0, 0, 0, 0};
int sequence[20] = {0};
// sequence: 1st index: level of difficulty, 2nd index: color of the led

int answer[20] = {0};
// Answer: 1st clue: level of difficulty, 2nd clue: color of the led

const int error_table[4][3] = {{1, 3, 2}, {0, 2, 0}, {3, 1, 3}, {2, 0, 1}};
// Error_table: 1st index: color led sequence, 2nd index: color led answer
// ROUGE : 0, BLEU : 1, VERT : 2, JAUNE : 3

int color[4][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
/*
  color: 1st index: led, 2nd index: color led (RGB)
  1: Up button
  2: Down button
  3: Left button
  4: Right button
*/

// Variable button delay definitions
int t_delay_isCliked = 140;
int t_delay_isPressed = 1500;
int t_delay_bounce = 120;

// Time variables
unsigned long int time_seq = 0;
unsigned long int time_ans = 0;
unsigned long int time_delay_led = 0;
int delay_seq = 2000;

int level = 0;
int level_max = 5;
int error = 0;
int sequence_number = 0;
int order_bt = 0;
int difficulty = 0;

// System variables
int state_system = INIT;
int state_game = 0;

void setup()
{
  lcd.begin(16, 2);
  setup_bt(4);

  // temporary assignment
  pinMode(5, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);

  setup_led(4);

  algo_led_random();
  algo_answer();

  Serial.begin(115200);
  delay(1000);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop()
{
  read_bt(4);

  strcpy(myData.a, "THIS IS A CHAR");
  myData.c = random(100, 200);
  myData.d = false;

  // Send message via ESP-NOW

  /*if (dataRecv.c != myData.c)
  {
    Serial.println("Received c");
    Serial.println(dataRecv.c);
    myData.c = dataRecv.c;
  }*/

  if (dataRecv.b != myData.b)
  {
    Serial.println("Received a");
    Serial.println(dataRecv.b);
    myData.b = dataRecv.b;
  }

  if (dataRecv.d != myData.d)
  {
    Serial.println("Received d");
    Serial.println(dataRecv.d);
    myData.d = dataRecv.d;
  }
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK)
  {
    Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }
  delay(2000);

  switch (state_system)
  {
  case INIT:
    // update_mini_game();
    led_PWM(JAUNE, led_pwm[JAUNE]);
    led_PWM(BLEU, led_pwm[BLEU]);
    if (bt[JAUNE].isPressed())
    {
      state_system = TEST;
      led_PWM_Off(JAUNE);
      led_PWM_Off(BLEU);
      // request(3, "1");
    }
    if (bt[BLEU].isPressed())
    {
      // request(3, "2");
      led_PWM_Off(JAUNE);
      led_PWM_Off(BLEU);
      state_system = GAME;
      state_game = ERR;
      delay(1000); // Wait 1 second
      time_seq = millis();
      time_delay_led = millis();
    }

    break;
  case TEST:
    // test_PWM();
    // test_all_led();
    smooth_RGB();
    break;
  case GAME:
    switch (state_game)
    {
    case SEQ:
      if (sequence_led(level))
      {
        if (sequence_number > level)
        {
          sequence_number = 0;
          time_seq = millis();
          time_ans = millis();
          state_game = ANS;
        }
      }
      break;
    case ANS:
      player_answer();
      break;
    case ERR:
      error_Answer(1000);
      break;
    case WIN:
      smooth_RGB();
      break;
    }
    break;
  }
}

void setup_bt(int nb_bt)
{
  /*
    Input : nb_bt : number of buttons (int)
    Output : none
    Description: This function is used to initialize the buttons
  */
  for (int k = 0; k < nb_bt; k++)
  {
    bt[k].begin(button_pin[k], LOW, t_delay_isCliked, t_delay_isPressed, t_delay_bounce);
  }
}

void read_bt(int nb_bt)
{
  /*
    Input : nb_bt : number of buttons (int)
    Output : none
    Description: This function is used to read the buttons
  */
  for (int k = 0; k < nb_bt; k++)
  {
    bt[k].read_Bt();
  }
}

void setup_led(int nb_led)
{
  /*
    Input : nb_led : number of leds (int)
    Output : none
    Description: This function is used to initialize the leds
  */
  for (int j = 0; j < 4; j++)
  {
    for (int i = 0; i < 3; i++)
    {
      ledcSetup(j * 3 + i, 5000, 10);
      ledcAttachPin(led_pin[j][i], j * 3 + i);
      ledcWrite(j, 0);
    }
  }
}

bool sequence_led(int num_seq)
{
  // This function allows you to make a sequence of leds according to the level of difficulty

  if (millis() - time_seq > delay_seq)
  {
    if (millis() - time_delay_led < 800)
    {
      reinitialize_color(sequence[sequence_number]);
      led_PWM(sequence[sequence_number], led_pwm[led_place[sequence[sequence_number]]]);
      return false;
    }
    else if (millis() - time_delay_led > 1300)
    {
      sequence_number++;
      time_delay_led = millis();
      return true;
    }
    else
    {
      led_PWM_Off(led_place[sequence[sequence_number]]);
      return false;
    }
  }
  else
  {
    time_delay_led = millis();
    return false;
  }
}

void player_answer()
{
  // This function makes it possible to check the order of the led sequence according to the response of the player
  for (int k = 0; k < 4; k++)
  {
    if (digitalRead(button_pin[k]) == LOW)
    {
      reinitialize_color(k);
      led_PWM(k, led_pwm[k]);
    }
    else
      led_PWM_Off(k);
  }

  if (millis() - time_ans > 10000)
  {
    for (int k = 0; k < 4; k++)
    {
      led_PWM_Off(k);
    }
    state_game = SEQ;
    time_seq = millis();
    order_bt = 0;
  }

  for (int k = 0; k < 4; k++)
  {
    if (bt[k].isCliked())
    {
      if (k == answer[order_bt])
      {
        order_bt++;
        for (int k = 0; k < 4; k++)
        {
          led_PWM_Off(k);
        }
        if (order_bt > level)
        {
          order_bt = 0;
          level++;
          state_game = SEQ;
          time_seq = millis();
          if (level >= level_max)
          {
            level = 0;
            state_system = WIN;
            for (int k = 0; k < 4; k++)
            {
              reinitialize_color(k);
            }
            algo_led_random();
            algo_answer();
          }
        }
      }
      else
      {
        error++;
        order_bt = 0;
        algo_led_random();
        algo_answer();
        time_seq = millis();
        time_delay_led = millis();
        state_game = ERR;
      }
    }
  }
}

void error_Answer(int delay)
{
  // This function allows all the LEDs to be turned on for a defined time.

  if (millis() - time_delay_led < delay)
  {
    for (int k = 0; k < 4; k++)
    {
      change_color(k, 1020, 1020, 1020);
      led_PWM(k, led_pwm[k]);
    }
  }
  else
  {
    for (int k = 0; k < 4; k++)
    {
      led_PWM_Off(k);
    }
    order_bt = 0;
    level = 0;
    if (error > 2)
    {
      error = 0;
      state_system = INIT;
      algo_led_random();
      algo_answer();
      for (int k = 0; k < 4; k++)
      {
        reinitialize_color(k);
      }
    }
    state_game = SEQ;
    time_seq = millis();
  }
}

void led_PWM(int led_num, int pwm[3])
{
  // This function allows you to make a sequence of leds according to the level of difficulty
  for (int i = 0; i < 3; i++)
  {
    ledcWrite(led_place[led_num] * 3 + i, pwm[i]);
  }
}

void led_PWM_Off(int led_num)
{
  // This function allows you to make a sequence of leds according to the level of difficulty
  for (int i = 0; i < 3; i++)
  {
    ledcWrite(led_place[led_num] * 3 + i, 0);
    led_pwm[led_place[led_num]][i] = 0;
  }
}

void smooth_RGB()
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 256; j++)
    {
      for (int k = 0; k < 4; k++)
      {
        led_pwm[k][i] = j;
        led_PWM(k, led_pwm[k]);
      }
      delay(1);
    }
    for (int j = 255; j >= 0; j--)
    {
      for (int k = 0; k < 4; k++)
      {
        led_pwm[k][i] = j;
        led_PWM(k, led_pwm[k]);
      }
      delay(1);
    }
  }

  for (int k = 0; k < 4; k++)
  {
    led_PWM_Off(k);
  }
}

void algo_led_random()
{
  // This function makes it possible to make a sequence of the colors of the random leds according to the level of the difficulty
  for (int k = 0; k < level_max; k++)
  {
    sequence[k] = random(0, 4);
  }
}

void algo_led_random_place(void)
{

  led_place[0] = random(0, 3);
  for (int i = 1; i < 4; i++)
  {
    led_place[i] = random(0, 3);
    for (int j = 0; j < i; j++)
    {
      if (led_place[i] == led_place[j])
      {
        i--;
        break;
      }
    }
  }
}

void algo_answer()
{
  // This function makes it possible to make the sequence of the response of the player according to the basic sequence and the error
  for (int k = 0; k < level_max; k++)
  {
    answer[k] = error_table[sequence[k]][error];
  }
}

void test_all_led(void)
{
  // test all led
  for (int i = 0; i < 3; i++)
  {
    ledcWrite(i, 500);
    ledcWrite(i + 3, 500);
    ledcWrite(i + 6, 500);
    ledcWrite(i + 9, 500);
    delay(1000);
    ledcWrite(i, 0);
    ledcWrite(i + 3, 0);
    ledcWrite(i + 6, 0);
    ledcWrite(i + 9, 0);
  }
}

void test_PWM(void)
{
  // test PWM with button
  for (int i = 0; i < 4; i++)
  {
    if (bt[i].isCliked())
    {
      reinitialize_color(i);
      led_PWM(i, led_pwm[i]);
      Serial.printf("bouton %d\n", i);
      // led_PWM(i, {500,0,0});
      delay(1000);
    }
    else
    {
      led_PWM_Off(i);
    }
  }
}

void reinitialize_color(int bt)
{
  // change color with button
  for (int i = 0; i < 3; i++)
  {
    led_pwm[led_place[bt]][i] = led_color[led_place[bt]][i];
  }
}

void change_color(int bt, int R, int G, int B)
{
  // change color with button
  led_pwm[bt][0] = R;
  led_pwm[bt][1] = G;
  led_pwm[bt][2] = B;
}

void rainbow_led()
{
  // Cette fonction génère une séquence de toutes les nuances de couleurs pour les leds afin de faire un arc-en-ciel
  // La premiere couleur est rouge, puis orange, puis jaune, puis vert, puis bleu, puis violet

  // génération de la séquence de couleur
}