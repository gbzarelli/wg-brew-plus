#include <max6675.h>
#include <Wire.h>
#include "MyDisplay.h"
#include "MySensors.h"

const int DELAY = 100;

const int M_PRINCIPAL = 0;
const int M_CONF_BRASSAGEM = 1;
const int M_CONF_FERVURA = 2;
const int M_BRASSAGEM = 3;
const int M_FERVURA = 4;

int PIN_BT_ENTER = 16;
int PIN_BT_ADD = 14;
int PIN_BT_SUB = 15;

int ups = 0;
int menu = 0;
int etapa = 0;
int sec = 0;
int indexTmp = 0;

struct config_brassagem {
  int tempPreAquec;
  int qtdRampas;
  int rampas[2][10];
};

struct config_fervura {
  int tempFervura;
  int tempoMin;
  int qtdLupulo;
  int lupulo[10];
};

int tmpBtPress = -1;
boolean tmpBtPressState = false;


// Variavel para controle de tempo UPS;
unsigned long time;

// Metodos
int getBtPress();
void refreshUPS();
void processBtPress(int btPress);
void processBrassagem();
void processFervura();

void setup() {
  Serial.begin(9600);
  setupDisplay();
  setupSensors();
  // Aguarda estabilização.
  delay(5000);
  updateMenuPrincipal(0);
}

/** ROTINA LOOP DO PROGRAMA.

   1 - BRASSAGEM
   tempo:    |--------Tn--------|------|T rampa 1|------|T rampa 2|  ...
   temp:     0º- > - > - > - > 50º - > 60º------60º - > 70º------70º ...
   timeline: /-pre-aquecimento-/------/--rampa1--/------/--rampa2--/-...
                               |                 |                 |
                               |-> Soa alarme ao fim dos t/pre e rampa.

   -> Ao chegar no fim do tempo da ultima rampa o sistema aguarda confirmação
   para iniciar fervura, enquanto não tem o 'ok' o rele de aquecimento é deslig.

   2 - FERVURA
   tempo:    |---------Tn--------|---------------T fervura--------------|
   temp:     xº - > - > - > - > 100º-----------------------------------100º
   timeline: /-aquecimento ferv-/------/-lup1-/--/-lup2-/-------/-lup3-/-
                                       |         |              |
                                       |->Soa alarme ao iniciar tempo de lupo;

*/
void loop() {
  time = millis();

  int btPressed = getBtPress();

  if (btPressed != -1) {
    processBtPress(btPressed);
  }
  if (menu == M_BRASSAGEM) {
    processBrassagem();
  } else if (menu == M_FERVURA) {
    processFervura();
  }

  refreshUPS();
}

void processBrassagem() {

}
void processFervura() {

}

/**
   Processamento das acoes dos botões / configuração do programa.
*/
void processBtPress(int btPress) {
  switch (menu) {
    case M_PRINCIPAL:
      if (btPress == PIN_BT_ADD) {
        indexTmp = 0;
      } else if (btPress == PIN_BT_SUB) {
        indexTmp = 1;
      } else if (btPress == PIN_BT_ENTER) {
        etapa = 0;
        if (indexTmp == 0) {
          menu = M_CONF_BRASSAGEM;
        } else {
          menu = M_CONF_FERVURA;
        }
      }
      updateMenuPrincipal(indexTmp);
      break;

    case M_CONF_BRASSAGEM:

      break;

    case M_CONF_FERVURA:

      break;
  }
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
    tmpBtPressState = false;
    return tmpBtPress;
  }
  return -1;
}

/**
   Metodo para controle de UPS.
*/
void refreshUPS() {
  if (ups > 1000) {
    ups = ups - 1000;
    sec++;
  } else {
    ups += DELAY;
  }

  time = millis() - time;
  if (time < DELAY) {
    delay(DELAY - time);
  } else {
    delay(time);
    ups += time;
  }
}

