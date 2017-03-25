#include "MyBuzzer.h"

//Constantes utilizadas para definir pino dos botoes
//e tambem define o botao que foi pressionado.
const int PIN_BT_ENTER = A5;
const int PIN_BT_ADD = 2;
const int PIN_BT_SUB = 3;

//Variaveis de controle de botoes pressionados
int tmpBtPress = -1;
boolean tmpBtPressState = false;
int secToBtPress;

void setupButtons();
int getBtPress();

void setupButtons(){
  pinMode(PIN_BT_ENTER, INPUT);
  pinMode(PIN_BT_ADD, INPUT);
  pinMode(PIN_BT_SUB, INPUT);
}

/**
   Retorna -1 se nenhum botão for pressionado.
*/
int getBtPress() { 
  if (digitalRead(PIN_BT_ENTER) == HIGH) {
    tmpBtPressState = true;
    tmpBtPress = PIN_BT_ENTER;
  } else if (digitalRead(PIN_BT_ADD) == HIGH) {
    tmpBtPressState = true;
    tmpBtPress = PIN_BT_ADD;
  } else if (digitalRead(PIN_BT_SUB) == HIGH) {
    tmpBtPressState = true;
    tmpBtPress = PIN_BT_SUB;
  } else if (tmpBtPressState) {
    beepButtonPressed();
    tmpBtPressState = false;
    secToBtPress=0;
    return tmpBtPress;
  }

  if(tmpBtPressState && secToBtPress++ > 10){
    beepButtonPressed();
    return tmpBtPress;  
  }
  
  return -1;
}

