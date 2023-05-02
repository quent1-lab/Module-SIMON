
#include <Arduino.h>
#include <stdio.h>
#include <inttypes.h>

#include "Bouton.h"



Bouton::Bouton()
{
    
}

void Bouton::begin(int pin, bool type_bt, int delay_click, int delay_press, int delay_rebond)
{
    PIN = pin;
    pinMode(pin,INPUT);
    DELAY_CLICK = delay_click;
    DELAY_PRESS = delay_press;
    DELAY_REBOND = delay_rebond;
    DELAY_RESET = 2500;
    TYPE = type_bt;
    TIME_BT = millis();
    STATE = 0;
}

void Bouton::read_Bt()
{
    timer_reset();
    if (d_read() == !TYPE && STATE == 0)
    {
        STATE = 1;
        reset();
    }
    if (d_read() == !TYPE && STATE == 1 && timer(DELAY_PRESS))
    {
        STATE = 3;
        return;
    }
    else if (d_read() == TYPE && STATE == 1 && timer(DELAY_CLICK))
    {
        STATE = 2;
        return;
    }
}

bool Bouton::isCliked()
{
    if (STATE == 2)
    {
        STATE = 0;
        return true;
    } else return false;
}

bool Bouton::isPressed()
{
    if (STATE == 3)
    {
        STATE = 0;
        reset();
        return true;
    } else return false;
}

int Bouton::d_read()
{
    return digitalRead(PIN);
}

void Bouton::reset()
{
    TIME_BT = millis();
}

bool Bouton::timer(int delay)
{
    if (millis() > TIME_BT + delay){
        return true;
    } else return false;
}

int Bouton::etat(){
    return STATE;
}

void Bouton::timer_reset(){
    if(STATE != 0 && timer(DELAY_RESET)){
        STATE = 0;
    }
}