#include "MyBuzzer.h"

//Constantes utilizadas para definir pino dos botoes
//e tambem define o botao que foi pressionado.

const int BT_1_ANALOG_VALUE=900;
const int BT_2_ANALOG_VALUE=800;
const int BT_3_ANALOG_VALUE=700;
const int BT_4_ANALOG_VALUE=500;
const int BT_5_ANALOG_VALUE=400;
const int BT_6_ANALOG_VALUE=250;
const int BT_7_ANALOG_VALUE=100;

const int BT_ENTER = BT_1_ANALOG_VALUE;
const int BT_ADD = BT_2_ANALOG_VALUE;
const int BT_SUB = BT_3_ANALOG_VALUE;
const int BT_RELE_1 = BT_4_ANALOG_VALUE;

const int PIN_BT=A0;


//Variaveis de controle de botoes pressionados
int tmpBtPress = -1;
boolean tmpBtPressState = false;
int secToBtPress;

void setupButtons();
int getBtPress();

void setupButtons(){
  pinMode(PIN_BT, INPUT);
}

/**
   Retorna -1 se nenhum botão for pressionado.
*/
int getBtPress() {
  int value=analogRead(PIN_BT);
  
  if (value>BT_1_ANALOG_VALUE) {
    tmpBtPressState = true;
    tmpBtPress = BT_ENTER;
  } else if (value>BT_2_ANALOG_VALUE) {
    tmpBtPressState = true;
    tmpBtPress = BT_ADD;
  } else if (value>BT_3_ANALOG_VALUE) {
    tmpBtPressState = true;
    tmpBtPress = BT_SUB;
  }  else if (value>BT_4_ANALOG_VALUE) {
    tmpBtPressState = true;
    tmpBtPress = BT_RELE_1;
  }else if (tmpBtPressState) {
    beepButtonPressed();
    tmpBtPressState = false;
    secToBtPress=0;
    return tmpBtPress;
  }

  if(tmpBtPressState && secToBtPress++ > 10 && BT_RELE_1 != tmpBtPress){
    beepButtonPressed();
    return tmpBtPress;  
  }
  
  return -1;
}

