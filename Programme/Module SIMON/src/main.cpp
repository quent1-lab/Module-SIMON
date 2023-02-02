#include <Arduino.h>
#include "bouton.h"

bouton bt[4];

void setup_bt(int nb_bt);

//Définitons variables des boutons
const int bt_Jaune = 4;
const int bt_Rouge = 13;
const int bt_Noir = 16;
const int bt_Vert = 17;
u_int8_t pin_Bouton[4] = {bt_Jaune, bt_Rouge, bt_Noir, bt_Vert};

//Définitons variables des lumières
const int led_Jaune = 21;
const int led_Rouge = 23;
const int led_Noir = 25;
const int led_Vert = 26;
u_int8_t pin_Led[4] = {led_Jaune, led_Rouge, led_Noir, led_Vert};

// Définitions variables delay des boutons
int t_delay_click = 140;
int t_delay_press = 1500;
int t_delay_rebond = 120;

#define JAUNE 0
#define ROUGE 1
#define NOIR 2
#define VERT 3

void setup() {
  setup_bt(4);

  for(int k = 0;k < 4;k++){
    pinMode(pin_Led[k],OUTPUT);
  }

}

void loop() {
  for(int k = 0;k<4;k++){
    bt[k].read_Bt();
  }

  if(bt[JAUNE].click()){
    digitalWrite(led_Jaune,HIGH);
    delay(500);
  }else digitalWrite(led_Jaune,LOW);

  if(bt[ROUGE].click()){
    digitalWrite(led_Rouge,HIGH);
    delay(500);
  }else digitalWrite(led_Rouge,LOW);

  if(bt[NOIR].click()){
    digitalWrite(led_Noir,HIGH);
    delay(500);
  }else digitalWrite(led_Noir,LOW);

  if(bt[VERT].press()){
    digitalWrite(led_Vert,HIGH);
    delay(500);
  }else digitalWrite(led_Vert,LOW);
}

void setup_bt(int nb_bt){
  for(int k = 0;k < nb_bt;k++){
    bt[k].begin(pin_Bouton[k],HIGH,t_delay_click,t_delay_press,t_delay_rebond);
  }
}
