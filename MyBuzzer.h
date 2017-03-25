#include "pitches.h"

const int PIN_BUZZER=10;

void beepButtonPressed();
void setupBuzzer();

void beepWaitConfirm();
void beepAlarmeRampaAsync();
void beepAlarmeLupuloAsync();
void beepAlarmeStartAsync();

int notes[] = {
  NOTE_A4, NOTE_B4, NOTE_C3
};

void setupBuzzer(){
  pinMode(PIN_BUZZER, OUTPUT);
}

void beepButtonPressed(){
  tone(PIN_BUZZER,notes[0],20);
}

void beepAlarmeLupuloAsync(){
  tone(PIN_BUZZER,notes[2],50);
}

void beepAlarmeStartAsync(){
  tone(PIN_BUZZER,notes[2],50);
}

void beepAlarmeRampaAsync(){
  tone(PIN_BUZZER,notes[2],50);
}

void beepWaitConfirm(){
  tone(PIN_BUZZER,notes[2],50);
}

