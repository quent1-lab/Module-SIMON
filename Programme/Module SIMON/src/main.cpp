/*
This mini game is a module of the video game "No Talking and nobody explodes"
The object of the game is to ANSroduce a sequence of LEDs by pressing the corresponding buttons in the defusing manual.
The game is made up of 4 buttons and 4 leds.
The JAUNE button launches the game.
Then the game generates a sequence of random LEDs.
The player must ANSroduce the sequence by pressing the corresponding buttons.
If the player succeeds in the sequence, he advances to the next level.
If the player fails, he must start the sequence again.
The game has 5 difficulty levels.
Level 1 is the easiest and level 5 is the most difficult.
The game is over when the player reaches level 5.
*/

//Add score for players

#include <Arduino.h>
#include "Bouton.h"
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "rgb_lcd.h"

Bouton bt[4];
HTTPClient client[4];
rgb_lcd lcd;

// Function prototype
void setup_bt(int nb_bt); 
void read_bt(int nb_bt);

void setup_led(int nb_led);
bool sequence_led(int level);
void algo_led_random(void);
void algo_led_random_place(void);
void algo_answer(void);
void player_answer(void);
void all_led_High(int delai);

void test_bt_led(void);

String get_Client(int nclient);
void json_mini_game();
void update_mini_game();
void server();
void request(int nclient, String data);

#define ROUGE 0
#define BLEU 1
#define VERT 2
#define JAUNE 3

#define PWMROUGE {1020,0,0}
#define PWMBLEU {0,0,1020}
#define PWMVERT {0,1020,0}
#define PWMJAUNE {1020,1020,0}

#define INIT 0
#define TEST 1
#define GAME 2

#define SEQ 0
#define ANS 1
#define ERR 2

// Definitions of WiFi variables
const char *ssid = "Asus_PA";
const char *pass = "SIMON_23";

//byte mac_addr[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

//Server variable definitions
String url[4] = { "http://192.168.137.1/database/json.php", 
                  "http://192.168.137.1/database/Start_py.bat",
                  "http://192.168.137.1/database/request.php?level=",
                  "http://192.168.137.1/database/request.php?mode_jeu="};
int mini_game[4] = {0, 0, 0, 0};
// mini_game: 1st index: id_module, 2nd index: level_max, 3rd index: level, 4th index: mode_jeu
int server_state_system = 0;
unsigned long int server_time = 0;

// Let's define button pins
const int bt_JAUNE = 4;
const int bt_ROUGE = 13;
const int bt_BLEU = 16;
const int bt_VERT = 17;
u_int8_t button_pin[4] = {bt_ROUGE, bt_BLEU, bt_VERT, bt_JAUNE};

// Let's define the pins of the buttons
const int pin_bt_BT1 = 34;
const int pin_bt_BT2 = 35;
const int pin_bt_BT3 = 32;
const int pin_bt_BT4 = 33;
u_int8_t button_pin2[4] = {pin_bt_BT1, pin_bt_BT2, pin_bt_BT3, pin_bt_BT4};

// Définitons des pins des lumières
const int led_JAUNE = 19;
const int led_ROUGE = 23;
const int led_BLEU = 25;
const int led_VERT = 26;
u_int8_t led_pin[4] = {led_ROUGE, led_BLEU, led_VERT, led_JAUNE};

//Let's define the pins of the leds
const int pin_led_BT1[3] = {23,22,21};
const int pin_led_BT2[3] = {19,18,5};
const int pin_led_BT3[3] = {17,16,4};
const int pin_led_BT4[3] = {2,15,0}; //Cour-circuit le pin sd1 vers GPIO 0

u_int8_t led_pin2[4][3] =  {{pin_led_BT1[0],pin_led_BT1[1],pin_led_BT1[2]}, 
                            {pin_led_BT2[0],pin_led_BT2[1],pin_led_BT2[2]}, 
                            {pin_led_BT3[0],pin_led_BT3[1],pin_led_BT3[2]}, 
                            {pin_led_BT4[0],pin_led_BT4[1],pin_led_BT4[2]}};
u_int8_t led_pwm[4][3] = {PWMROUGE, PWMBLEU, PWMVERT, PWMJAUNE};
u_int8_t led_place[4] = {ROUGE, BLEU, VERT, JAUNE};


// Variable button delay definitions
int t_delay_click = 140;
int t_delay_press = 1500;
int t_delay_bounce = 120;

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

// Time variables
unsigned long int time_seq = 0;
unsigned long int time_ans = 0;
unsigned long int time_delay_led = 0;
int delay_seq = 2000;

int level = 0;
int level_max = 20;
int error = 0;
int sequence_number = 0;
int order_bt = 0;

// System variables
int state_system = 0;
int state_game = 0;

void setup()
{
  lcd.begin(16, 2);
  setup_bt(4);

  for (int k = 0; k < 4; k++)
  {
    pinMode(led_pin[k], OUTPUT);
  }

  // temporary assignment
  pinMode(5, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);

  setup_led(4);

  algo_led_random();
  algo_answer();

  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA); // Optional
  WiFi.begin(ssid, pass);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  client[0].begin(url[0]);
  client[1].begin(url[1]);

  request(2, "0");
  request(3, "0");
  delay(2000);
}

void loop()
{
  read_bt(4);
  server();

  switch (state_system)  {
  case INIT:
    update_mini_game();
    if (bt[JAUNE].click() || mini_game[3] == 1){
      state_system = TEST;
      request(3, "1");
    }
    if (bt[BLEU].click() || mini_game[3] == 2)
    {
      request(3, "2");
      state_system = GAME;
      state_game = ERR;
      time_seq = millis();
      time_delay_led = millis();
    }

    break;
  case TEST:
    test_bt_led();
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
      all_led_High(1000);     
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
    bt[k].begin(button_pin[k], HIGH, t_delay_click, t_delay_press, t_delay_bounce);
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

String get_Client(int nclient)
{
  /*
    Input : nclient : customer number (int)
    Output : payload : customer data (String)
    Description: This function is used to retrieve customer data
  */
  int httpCode = client[nclient].GET();

  if (httpCode > 0)
  {
    return client[nclient].getString();    
  }
  else
  {
    Serial.println("Error on HTTP request");
    return "0";
  }
}

void json_mini_game(){
  /*
    Input : none
    Output : none
    Description: This function is used to recover the data of the mini game
  */
  String payload = get_Client(0);

  if (payload == "0")
    return;

  char json[payload.length() + 1];
  payload.replace(" ","");
  payload.replace("\\","");
  payload.trim();
  payload.remove(0,1);
  payload.toCharArray(json, payload.length() + 1);
  Serial.println(json);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, json);

  mini_game[0] = doc[0]["id_module"];
  mini_game[1] = doc[0]["nv_max"];
  mini_game[2] = doc[0]["nv"];
  mini_game[3] = doc[0]["mode_jeu"];
}

void update_mini_game(){
  if(mini_game[1] != level_max){
    level_max = mini_game[1];
    algo_led_random();
    algo_answer();
  }
  if(mini_game[3] != state_system){
    //state_system = mini_game[3];
  }
}

void server(){
  
  switch(server_state_system){
    case 0:
      //get_Client(1);
      server_state_system = 1;
      server_time = millis();
      break;
    case 1 :
      //wait 100ms to avoid errors
      if(millis() - server_time > 100){
        server_state_system = 2;
      }
      break;
    case 2 :
      json_mini_game();
      server_state_system = 3;
      break;
    case 3 :
      if(state_system == 0){
        server_state_system = 0;
      }
    default :
      break;
  }
}

void request(int nclient, String data)
{
  /*
    Input : nclient : customer number (int)
            data : data to send (String)
    Output : none
    Description: This function is used to send data to the client
  */
  url[nclient] += data;
  client[nclient].begin(url[nclient]);
  client[nclient].GET();
  url[nclient].remove(url[nclient].length() - 1);
  client[nclient].end();
}

void setup_led(int nb_led)
{
  /*
    Input : nb_led : number of leds (int)
    Output : none
    Description: This function is used to initialize the leds
  */
  for(int j = 0; j < 4; j++)
  {
    for(int i = 0; i < 3; i++)
    {
      ledcSetup(j*3+i, 5000, 8);
      ledcAttachPin(led_pin2[j][i], j*3+i);
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
      digitalWrite(led_pin[sequence[sequence_number]], HIGH);
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
      digitalWrite(led_pin[sequence[sequence_number]], LOW);
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
      digitalWrite(led_pin[k], HIGH);
    }
    else
      digitalWrite(led_pin[k], LOW);
  }

  if (millis() - time_ans > 10000)
  {
    for (int k = 0; k < 4; k++)
    {
      digitalWrite(led_pin[k], LOW);
    }
    state_game = SEQ;
    time_seq = millis();
    order_bt = 0;
  }

  for (int k = 0; k < 4; k++)
  {
    if (bt[k].click())
    {
      if (k == answer[order_bt])
      {
        order_bt++;
        if (order_bt > level)
        {
          order_bt = 0;
          level++;
          state_game = SEQ;
          time_seq = millis();
          if (level >= level_max)
          {
            request(3, "0");
            level = 0;
            state_system = INIT;
            algo_led_random();
            algo_answer();
          }
          request(2, String(level));
        }
      }
      else
      {
        error++;
        order_bt = 0;
        algo_answer();
        time_seq = millis();
        time_delay_led = millis();
        state_game = ERR;
      }
    }
  }
}

void all_led_High(int delay)
{
  // This function allows all the LEDs to be turned on for a defined time.

  if (millis() - time_delay_led < delay)
  {
    for (int k = 0; k < 4; k++)
    {
      digitalWrite(led_pin[k], HIGH);
    }
  }
  else
  {
    for (int k = 0; k < 4; k++)
    {
      digitalWrite(led_pin[k], LOW);
    }
    order_bt = 0;
    level = 0;
    if (error > 2)
    {
      error = 0;
      state_system = INIT;
      algo_led_random();
      algo_answer();
    }
    state_game = SEQ;
    time_seq = millis();
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
  for(int i = 1; i < 4; i++)
  {
    led_place[i] = random(0, 3);
    for(int j = 0; j < i; j++)
    {
      if(led_place[i] == led_place[j])
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

void test_bt_led()
{
  // This function is used to test buttons and leds
  // If the button is pressed, the corresponding LED lights up
  // If the button is released, the corresponding LED turns off
  if (bt[JAUNE].click())
  {
    digitalWrite(led_JAUNE, HIGH);
    led_time[JAUNE] = millis();
  }
  else
  {
    if (millis() > led_time[JAUNE] + 500)
    {
      digitalWrite(led_JAUNE, LOW);
    }
  }

  if (bt[ROUGE].click())
  {
    digitalWrite(led_ROUGE, HIGH);
    led_time[ROUGE] = millis();
  }
  else
  {
    if (millis() > led_time[ROUGE] + 500)
    {
      digitalWrite(led_ROUGE, LOW);
    }
  }

  if (bt[BLEU].click())
  {
    digitalWrite(led_BLEU, HIGH);
    led_time[BLEU] = millis();
  }
  else
  {
    if (millis() > led_time[BLEU] + 500)
    {
      digitalWrite(led_BLEU, LOW);
    }
  }

  if (bt[VERT].press())
  {
    digitalWrite(led_VERT, HIGH);
    led_time[VERT] = millis();
  }
  else
  {
    if (millis() > led_time[VERT] + 500)
    {
      digitalWrite(led_VERT, LOW);
    }
  }
}