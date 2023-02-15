/*
Ce mini jeu est un module du jeu vidéo "No Talking and nobody explodes"
Le but du jeu est de reproduire une séquence de led en appuyant sur les boutons correspondants au manuel de désamorsage.
Le jeu est composé de 4 boutons et de 4 leds.
Le bouton jaune permet de lancer le jeu.
Ensuite le jeu génère une séquence de led aléatoire.
Le joueur doit reproduire la séquence en appuyant sur les boutons correspondants.
Si le joueur réussi la séquence, il passe au niveau suivant.
Si le joueur échoue, il doit recommencer la séquence.
Le jeu est composé de 5 niveaux de difficulté.
Le niveau 1 est le plus facile et le niveau 5 est le plus difficile.
Le jeu est terminé quand le joueur réussi le niveau 5.
*/

// Rajouter une option pour connecter le jeu sur un site web pour enregistrer les scores des joueurs

#include <Arduino.h>
#include "Bouton.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"

Bouton bt[4];
HTTPClient client[4];

// Prototype des fonctions
void setup_bt(int nb_bt);
void read_bt(int nb_bt);
void test_bt_led(void);
bool sequence_led(int niveau);
void algo_led_aleatoire(void);
void algo_reponse(void);
void reponse_bt(void);
void all_led_High(int delai);

String get_Client(int nclient);
void json_mini_jeu();
void update_mini_jeu();
void server();
void request(int nclient, String data);

#define ROUGE 0
#define NOIR 1
#define VERT 2
#define JAUNE 3

#define INIT 0
#define TEST 1
#define JEU1 2

#define SEQ 0
#define REP 1
#define ERR 2

// Définitions des variables WiFi
const char *ssid = "Asus_PA";
const char *pass = "SIMON_23";

//byte mac_addr[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

//Définitions des variables pour le serveur
String url[4] = { "http://192.168.137.1/database/json.php", 
                  "http://192.168.137.1/database/Start_py.bat",
                  "http://192.168.137.1/database/index.php?niveau=",
                  "http://192.168.137.1/database/index.php?mode_jeu="};
int mini_jeu[4] = {0, 0, 0, 0};
// mini_jeu : 1er indice : id_module, 2eme indice : niveau_max, 3eme indice : niveau, 4eme indice : mode_jeu
int etat_serveur = 0;
unsigned long int time_serveur = 0;

// Définitons des pins des boutons
const int bt_Jaune = 4;
const int bt_Rouge = 13;
const int bt_Noir = 16;
const int bt_Vert = 17;
u_int8_t pin_Bouton[4] = {bt_Rouge, bt_Noir, bt_Vert, bt_Jaune};

// Définitons des pins des lumières
const int led_Jaune = 21;
const int led_Rouge = 23;
const int led_Noir = 25;
const int led_Vert = 26;
u_int8_t pin_Led[4] = {led_Rouge, led_Noir, led_Vert, led_Jaune};

// Définitions variables delay des boutons
int t_delay_click = 140;
int t_delay_press = 1500;
int t_delay_rebond = 120;

// Variables led et boutons
unsigned long int time_led[4] = {0, 0, 0, 0};
int couleur_bt[4] = {0, 0, 0, 0};
int sequence[20] = {0};
// sequence : 1er indice : niveau de la difficulté, 2eme indice : couleur de la led

int reponse[20] = {0};
// reponse : 1er indice : niveau de la difficulté, 2eme indice : couleur de la led

const int tableau_err[4][3] = {{1, 3, 2}, {0, 2, 0}, {3, 1, 3}, {2, 0, 1}};
// tableau_err : 1er indice : couleur led sequence, 2eme indice : couleur led reponse
// ROUGE : 0, NOIR : 1, VERT : 2, JAUNE : 3

int couleur[4][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
/*
  couleur : 1er indice : led, 2eme indice : couleur led (RGB)
  1 : Bouton haut
  2 : Bouton bas
  3 : Bouton gauche
  4 : Bouton droite
*/

// Variables temps
unsigned long int time_seq = 0;
unsigned long int time_rep = 0;
unsigned long int time_delay_led = 0;
int delai_seq = 2000;

int niveau = 0;
int niveau_max = 20;
int erreur = 0;
int numero_seq = 0;
int ordre_bt = 0;

// Variables systèmes
int etat = 0;
int etat_jeu1 = 0;

void setup()
{
  setup_bt(4);

  for (int k = 0; k < 4; k++)
  {
    pinMode(pin_Led[k], OUTPUT);
  }

  // assignement temporaire
  pinMode(5, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);

  algo_led_aleatoire();
  algo_reponse();

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

  switch (etat)  {
  case INIT:
    update_mini_jeu();
    if (bt[JAUNE].click() || mini_jeu[3] == 1){
      etat = TEST;
      request(3, "1");
    }
    if (bt[NOIR].click() || mini_jeu[3] == 2)
    {
      request(3, "2");
      etat = JEU1;
      etat_jeu1 = ERR;
      time_seq = millis();
      time_delay_led = millis();
    }

    break;
  case TEST:
    test_bt_led();
    break;
  case JEU1:
    switch (etat_jeu1) // ajouter etat erreur
    {
    case SEQ:
      if (sequence_led(niveau))
      {
        if (numero_seq > niveau)
        {
          numero_seq = 0;
          time_seq = millis();
          time_rep = millis();
          etat_jeu1 = REP;
        }
      }
      break;
    case REP:
      reponse_bt();

      break;
    case ERR:
      all_led_High(1000);
      break;
    }
    break;
  }
  //Serial.printf("etat : %d, etat_jeu1 : %d, niveau : %d, erreur : %d, numero_seq : %d, ordre_bt : %d\n", etat, etat_jeu1, niveau, erreur, numero_seq, ordre_bt);
}

void setup_bt(int nb_bt)
{
  /*
    Input : nb_bt : nombre de boutons (int)
    Output : none
    Description : Cette fonction permet d'initialiser les boutons
  */
  for (int k = 0; k < nb_bt; k++)
  {
    bt[k].begin(pin_Bouton[k], HIGH, t_delay_click, t_delay_press, t_delay_rebond);
  }
}

void read_bt(int nb_bt)
{
  /*
    Input : nb_bt : nombre de boutons (int)
    Output : none
    Description : Cette fonction permet de lire les boutons
  */
  for (int k = 0; k < nb_bt; k++)
  {
    bt[k].read_Bt();
  }
}

String get_Client(int nclient)
{
  /*
    Input : nclient : numéro du client (int)
    Output : payload : données du client (String)
    Description : Cette fonction permet de récupérer les données du client
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

void json_mini_jeu(){
  /*
    Input : none
    Output : none
    Description : Cette fonction permet de récupérer les données du mini jeu
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

  mini_jeu[0] = doc[0]["id_module"];
  mini_jeu[1] = doc[0]["nv_max"];
  mini_jeu[2] = doc[0]["nv"];
  mini_jeu[3] = doc[0]["mode_jeu"];
}

void update_mini_jeu(){
  if(mini_jeu[1] != niveau_max){
    niveau_max = mini_jeu[1];
    algo_led_aleatoire();
    algo_reponse();
  }
  if(mini_jeu[3] != etat){
    //etat = mini_jeu[3];
  }
}

void server(){
  
  switch(etat_serveur){
    case 0:
      //get_Client(1);
      etat_serveur = 1;
      time_serveur = millis();
      break;
    case 1 :
      //attente de 100ms pour éviter les erreurs
      if(millis() - time_serveur > 100){
        etat_serveur = 2;
      }
      break;
    case 2 :
      json_mini_jeu();
      etat_serveur = 3;
      break;
    case 3 :
      if(etat == 0){
        etat_serveur = 0;
      }
    default :
      break;
  }
}

void request(int nclient, String data)
{
  /*
    Input : nclient : numéro du client (int)
            data : données à envoyer (String)
    Output : none
    Description : Cette fonction permet d'envoyer des données au client
  */
  //client[nclient].addHeader("Content-Type", "application/json");
  url[nclient] += data;
  client[nclient].begin(url[nclient]);
  client[nclient].GET();
  url[nclient].remove(url[nclient].length() - 1);
}

bool sequence_led(int num_seq)
{
  // Cette fonction permet de faire une séquence de led en fonction du niveau de la difficulté

  if (millis() - time_seq > delai_seq)
  {
    if (millis() - time_delay_led < 800)
    {
      digitalWrite(pin_Led[sequence[numero_seq]], HIGH);
      return false;
    }
    else if (millis() - time_delay_led > 1300)
    {
      numero_seq++;
      time_delay_led = millis();
      return true;
    }
    else
    {
      digitalWrite(pin_Led[sequence[numero_seq]], LOW);
      return false;
    }
  }
  else
  {
    time_delay_led = millis();
    return false;
  }
}

void reponse_bt()
{
  // Cette fonction permet de vérifier l'ordre de la séquence led en fonction de la réponse du joueur
  for (int k = 0; k < 4; k++)
  {
    if (digitalRead(pin_Bouton[k]) == LOW)
    {
      digitalWrite(pin_Led[k], HIGH);
    }
    else
      digitalWrite(pin_Led[k], LOW);
  }

  if (millis() - time_rep > 10000)
  {
    for (int k = 0; k < 4; k++)
    {
      digitalWrite(pin_Led[k], LOW);
    }
    etat_jeu1 = SEQ;
    time_seq = millis();
    ordre_bt = 0;
  }

  for (int k = 0; k < 4; k++)
  {
    if (bt[k].click())
    {
      if (k == reponse[ordre_bt])
      {
        ordre_bt++;
        if (ordre_bt > niveau)
        {
          ordre_bt = 0;
          niveau++;
          etat_jeu1 = SEQ;
          time_seq = millis();
          if (niveau >= niveau_max)
          {
            request(3, "0");
            niveau = 0;
            etat = INIT;
            algo_led_aleatoire();
            algo_reponse();
          }
          request(2, String(niveau));
        }
      }
      else
      {
        erreur++;
        ordre_bt = 0;
        algo_reponse();
        time_seq = millis();
        time_delay_led = millis();
        etat_jeu1 = ERR;
      }
    }
  }
}

void all_led_High(int delai)
{
  // Cette fonction permet d'allumer toutes les leds pendant un temps défini

  if (millis() - time_delay_led < delai)
  {
    for (int k = 0; k < 4; k++)
    {
      digitalWrite(pin_Led[k], HIGH);
    }
  }
  else
  {
    for (int k = 0; k < 4; k++)
    {
      digitalWrite(pin_Led[k], LOW);
    }
    ordre_bt = 0;
    niveau = 0;
    if (erreur > 2)
    {
      erreur = 0;
      etat = INIT;
      algo_led_aleatoire();
      algo_reponse();
    }
    etat_jeu1 = SEQ;
    time_seq = millis();
  }
}

void algo_led_aleatoire()
{
  // Cette fonction permet de faire une séquence de la couleurs des leds aléatoire en fonction du niveau de la difficulté
  for (int k = 0; k < niveau_max; k++)
  {
    sequence[k] = random(0, 4);
  }
}

void algo_reponse()
{
  // Cette fonction permet de faire la séquence de la réponse du joueur en fonction de la séquence de base et de l'erreur
  for (int k = 0; k < niveau_max; k++)
  {
    reponse[k] = tableau_err[sequence[k]][erreur];
  }
}

void test_bt_led()
{
  // Cette fonction permet de tester les boutons et les leds
  // Si le bouton est appuyé, la led correspondante s'allume
  // Si le bouton est relaché, la led correspondante s'éteint
  if (bt[JAUNE].click())
  {
    digitalWrite(led_Jaune, HIGH);
    time_led[JAUNE] = millis();
  }
  else
  {
    if (millis() > time_led[JAUNE] + 500)
    {
      digitalWrite(led_Jaune, LOW);
    }
  }

  if (bt[ROUGE].click())
  {
    digitalWrite(led_Rouge, HIGH);
    time_led[ROUGE] = millis();
  }
  else
  {
    if (millis() > time_led[ROUGE] + 500)
    {
      digitalWrite(led_Rouge, LOW);
    }
  }

  if (bt[NOIR].click())
  {
    digitalWrite(led_Noir, HIGH);
    time_led[NOIR] = millis();
  }
  else
  {
    if (millis() > time_led[NOIR] + 500)
    {
      digitalWrite(led_Noir, LOW);
    }
  }

  if (bt[VERT].press())
  {
    digitalWrite(led_Vert, HIGH);
    time_led[VERT] = millis();
  }
  else
  {
    if (millis() > time_led[VERT] + 500)
    {
      digitalWrite(led_Vert, LOW);
    }
  }
}

//

// Rajouter timer pour les led
// Faire fonction pour le controle des led
// Optimiser la librairie bouton