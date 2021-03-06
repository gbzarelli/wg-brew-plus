#include "pitches.h"
#define PIN_BUZZER 11

void beepButtonPressed();
void setupBuzzer();

void alarmAsync();
void alarmWaitConfirm();
void playAsync();
void stopAlarm();
int timeToReproduction;

int notes[] = {
  NOTE_A4, NOTE_B4
};

long lastTime = -1;

void loopBuzzer(){
  if(timeToReproduction>0){
    if(lastTime<0){
      lastTime = millis();
    }
    playAsync();
    timeToReproduction = timeToReproduction-(millis() - lastTime);
    lastTime = millis();
  }else{
    lastTime=-1;
  }
}

void stopAlarm(){
  timeToReproduction=-1;
}

void setupBuzzer(){
  pinMode(PIN_BUZZER, OUTPUT);
}

void beepButtonPressed(){
  tone(PIN_BUZZER,notes[0],20);
}

void alarmAsync(int duration){
  timeToReproduction=duration * 1000;
}

void playAsync(){
  tone(PIN_BUZZER,notes[1],50);
}

void alarmWaitConfirm(){
  tone(PIN_BUZZER,notes[1],50);
}

