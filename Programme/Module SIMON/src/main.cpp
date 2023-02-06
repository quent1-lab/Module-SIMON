#include <Arduino.h>
#include "Bouton.h"

Bouton bt[4];

void setup_bt(int nb_bt);
void read_bt(int nb_bt);
void test_bt_led(void);
void sequence_led(int niveau);

#define JAUNE 0
#define ROUGE 1
#define NOIR 2
#define VERT 3

#define INIT 0
#define TEST 1

//Définitons des pins des boutons
const int bt_Jaune = 4;
const int bt_Rouge = 13;
const int bt_Noir = 16;
const int bt_Vert = 17;
u_int8_t pin_Bouton[4] = {bt_Jaune, bt_Rouge, bt_Noir, bt_Vert};

//Définitons des pins des lumières
const int led_Jaune = 21;
const int led_Rouge = 23;
const int led_Noir = 25;
const int led_Vert = 26;
u_int8_t pin_Led[4] = {led_Jaune, led_Rouge, led_Noir, led_Vert};

// Définitions variables delay des boutons
int t_delay_click = 140;
int t_delay_press = 1500;
int t_delay_rebond = 120;

//Variables led
unsigned long int time_led[4] = {0,0,0,0};

//Variables systèmes
int etat = 0;

void setup() {
  setup_bt(4);

  for(int k = 0;k < 4;k++){
    pinMode(pin_Led[k],OUTPUT);
  }

}

void loop() {
  switch(etat){
    case INIT:
      read_bt(4);
      if(bt[JAUNE].click()) etat = TEST;
      break;
    case TEST:
      read_bt(4);
      test_bt_led();
      break;
  }
}

void setup_bt(int nb_bt){
  for(int k = 0;k < nb_bt;k++){
    bt[k].begin(pin_Bouton[k],HIGH,t_delay_click,t_delay_press,t_delay_rebond);
  }
}

void read_bt(int nb_bt){
  for(int k = 0;  k< nb_bt; k++){
      bt[k].read_Bt();
  }
}

void sequence_led(int niveau){
//Cette fonction permet de faire une séquence de led
//Elle prend en paramètre le niveau de la séquence
if(niveau == 1){
}
}

void test_bt_led(){
  if(bt[JAUNE].click()){
      digitalWrite(led_Jaune,HIGH);
      time_led[JAUNE] = millis();
    }else {
      if(millis() > time_led[JAUNE] + 500)
      {
        digitalWrite(led_Jaune,LOW);
      }
    }

    if(bt[ROUGE].click()){
      digitalWrite(led_Rouge,HIGH);
      time_led[ROUGE] = millis();
    }else {
      if(millis() > time_led[ROUGE] + 500)
      {
        digitalWrite(led_Rouge,LOW);
      }
    }

    if(bt[NOIR].click()){
      digitalWrite(led_Noir,HIGH);
      time_led[NOIR] = millis();
    }else {
      if(millis() > time_led[NOIR] + 500)
      {
        digitalWrite(led_Noir,LOW);
      }
    }

    if(bt[VERT].press()){
      digitalWrite(led_Vert,HIGH);
      time_led[VERT] = millis();
    }else {
      if(millis() > time_led[VERT] + 500)
      {
        digitalWrite(led_Vert,LOW);
      }
    }
}

//Rajouter timer pour les led
//Faire fonction pour le controle des led
//Optimiser la librairie bouton