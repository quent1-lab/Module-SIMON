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
const int led_Jaune = 22;
const int led_Rouge = 23;
const int led_Noir = 25;
const int led_Vert = 26;
u_int8_t pin_Led[4] = {led_Jaune, led_Rouge, led_Noir, led_Vert};

// Définitions variables delay des boutons
int t_delay_click = 200;
int t_delay_press = 3000;
int t_delay_rebond = 200;

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
  if(bt[JAUNE].click()){
    digitalWrite(bt_Jaune,HIGH);
  }else digitalWrite(bt_Jaune,LOW);

  if(bt[ROUGE].click()){
    digitalWrite(bt_Jaune,HIGH);
  }else digitalWrite(bt_Jaune,LOW);

  if(bt[NOIR].click()){
    digitalWrite(bt_Jaune,HIGH);
  }else digitalWrite(bt_Jaune,LOW);

  if(bt[VERT].click()){
    digitalWrite(bt_Jaune,HIGH);
  }else digitalWrite(bt_Jaune,LOW);
}

void setup_bt(int nb_bt){
  for(int k = 0;k < nb_bt;k++){
    bt[k].begin(pin_Bouton[k],HIGH,t_delay_click,t_delay_press,t_delay_rebond);
  }
}
