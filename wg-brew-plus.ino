#include <max6675.h>
#include <Wire.h>
#include "MyDisplay.h"
#include "MySensors.h"

//Delay de cada fim de ciclo do loop()
const int DELAY = 100;

//Constantes que define a tela atual a ser exibida.
const int M_PRINCIPAL = 0;
const int M_CONF_BRASSAGEM = 1;
const int M_CONF_FERVURA = 2;
const int M_BRASSAGEM = 3;
const int M_FERVURA = 4;

//Constantes utilizadas para definir pino dos botoes
//e tambem define o botao que foi pressionado.
const int PIN_BT_ENTER = 16;
const int PIN_BT_ADD = 14;
const int PIN_BT_SUB = 15;

//Constantes utilizadas na configuração da brassagem e fervura
const int ETAPA_CONF_TEMP_FERV = 0;
const int ETAPA_CONF_DURC_FERV = 1;
const int ETAPA_CONF_QTD_LUPULO = 2;
const int ETAPA_CONF_TEMP_PREAQC_BRASS = 0;
const int ETAPA_CONF_QTD_RAMPAS = 1;

//Constantes para processo de brassagem e fervura (execucao)
const int ETAPA_PREAQUEC=0;
const int ETAPA_RAMPAS=1;
const int ETAPA_WAIT_CONFIRM_FERV=2;
const int ETAPA_FERVER=3;
const int ETAPA_FERVURA_LUP=4;
const int ETAPA_WAIT_CONFIRM_END=5;

// Constanstes de configuracao de tempo;
const int SEC_ALARM_LUPULO = 5;//duração do alarme do lupulo

//Variaveis de controle
int ups = 0;//controla o ups (update por segundo) de atualização da tela.
int menu = 0;
int etapa = 0;
int sec = 0;//controle de segundos.
int indexTmp = 0;//indice temporario.(utilizado em qualquer rotina)
int menuSelectTmp;//controle de qual item do menu foi selecionado.
boolean startTimer;//controle para definir o inicio do timer;

//*************************************************************
// Estrutura de configuração das etapas de brassagem e fervura
struct config_brassagem {
  int tempPreAquec;
  int qtdRampas;
  int rampas[10][2];//[0,0]=temp;[0,1]=durac
};

struct config_fervura {
  int tempFervura;
  int duracaoMin;
  int qtdLupulo;
  int lupulo[10];
};

struct config_fervura fervura;
struct config_brassagem brassagem;
//*************************************************************

//Variaveis de controle de botoes pressionados
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

  brassagem.tempPreAquec = 50;
  brassagem.qtdRampas = 10;
  for (int i = 0 ; i < 10; i++) {
    for (int b = 0; b < 2; b++) {
      brassagem.rampas[i][b] = 0;
    }
  }

  fervura.tempFervura = 100;
  fervura.duracaoMin = 60;
  fervura.qtdLupulo = 5;
  for (int i = 0 ; i < 10; i++) {
    fervura.lupulo[i] = 0;
  }
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
   timeline: /-aquecimento ferv-/------/-lup1----/-lup2---------/-lup3---
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
  int temperature;
  int duration;
  
  switch(etapa){
    //**********************************
    case ETAPA_PREAQUEC:
      if(getThermoC() < brassagem.tempPreAquec){
        refreshResistence(brassagem.tempPreAquec);
      }else{
        startTimer=false;
        etapa=ETAPA_RAMPAS;
        indexTmp=0;
      }
      updatePreAquecBrassagem((int)getThermoC(),brassagem.tempPreAquec);
    break;

    //**********************************
    case ETAPA_RAMPAS:
      temperature = brassagem.rampas[indexTmp][0];
      duration = brassagem.rampas[indexTmp][1] * 60;
     
      //se o chronometro nao foi iniciado e a temperatura chegou no nivel da rampa,
      //inicia o chronometro;
      if(!startTimer && getThermoC() >= temperature) {
        startTimer=true;
        sec=0;
      }
      
      if(startTimer && sec >= duration){
        //Verifica se ainda existe rampas configuradas:
        if(indexTmp < brassagem.qtdRampas){
          indexTmp++;//Se existir incrementa;
        }else{
          indexTmp=0;
          etapa = ETAPA_WAIT_CONFIRM_FERV;
        }
        startTimer=false;//para chrono para atingir temperatura da prox rampa;
        break;
      }

      //Se o timer já iniciou comeca a decrementar o tempo da rampa
      //para exibição.
      if(startTimer){
        duration = duration - sec;
      }

      updateRampa(startTimer, (indexTmp+1), brassagem.qtdRampas,getThermoC(), temperature, duration);
      refreshResistence(temperature);
      
      break;

    //************************************************************
    //******AGUARDANDO CONFIRMACAO PARA INICIO DE FERCURA**********
    case ETAPA_WAIT_CONFIRM_FERV:
      turnOffResistence();//DESLIGA RESISTENCIA
      updateWaitConfirmFerv();
    break;
  }

}

void processFervura() {
    int alarmTime=-1;
    switch(etapa){
      //**********************************
      case ETAPA_PREAQUEC:
        if(getThermoC() >= fervura.tempFervura){
          sec=0;
          indexTmp=0;
          etapa=ETAPA_FERVURA_LUP;
        }
        updatePreAquecFervura((int)getThermoC(),fervura.tempFervura);
      break;
      
      //**********************************
      case ETAPA_FERVURA_LUP:
        alarmTimeStart = fervura.lupulo[indexTmp] * 60;
        alarmTimeEnd = alarmTimeStart + SEC_ALARM_LUPULO;

        //Fim do tempo de fervura!
        if(startTimer && sec >= (fervura.duracaoMin*60)){
          indexTmp=0;
          etapa=ETAPA_WAIT_CONFIRM_END;
          break;
        }

        if(sec >= alarmTimeStart && sec <=alarmTimeEnd){
          //TODO disparar alarme sonoro.
        }else if(sec > alarmTimeEnd){
          //TODO PARAR ALARME
          if((indexTmp+1) < fervura.qtdLupulo){
            indexTmp++;
          }
        }
        updateFervura((int)getThermoC(),fervura.tempFervura,(indexTmp+1),fervura.qtdLupulo,sec);
      break;
      //************************************************************
      //******AGUARDANDO CONFIRMACAO PARA INICIO DE FERCURA**********
      case ETAPA_WAIT_CONFIRM_END:
        turnOffResistence();//DESLIGA RESISTENCIA
        updateWaitConfirmEnd();
        return;
      break;
    }
    refreshResistence(brassagem.tempFervura);
}

/**
   Processamento das acoes dos botões / configuração do programa.
     switch(btPress){
       case PIN_BT_ENTER:
       break;
       case PIN_BT_ADD:
       break;
       case PIN_BT_SUB:
       break;
     }
*/
void processBtPress(int btPress) {
  switch (menu) {
    /*=============================================*/
    /*=============menu principal==================*/
    case M_PRINCIPAL:
      switch (btPress) {
        case PIN_BT_ENTER:
          indexTmp = 0;
          if (indexTmp == 0) {
            etapa = ETAPA_CONF_TEMP_PREAQC_BRASS;
            menu = M_CONF_BRASSAGEM;
          } else {
            etapa = ETAPA_CONF_TEMP_FERV;
            menu = M_CONF_FERVURA;
          }
          menuSelectTmp = menu;
          break;

        case PIN_BT_ADD:
          indexTmp = 0;
          break;

        case PIN_BT_SUB:
          indexTmp = 1;
          break;
      }
      updateMenuPrincipal(indexTmp);
      break;
    /*=============================================*/
    /*=============menu conf. brassagem============*/
    case M_CONF_BRASSAGEM:
      if (btPress == PIN_BT_ENTER) {
        if (etapa == (brassagem.qtdRampas * 2 + 2 )) { //quantidade de rampas*2(temp+time) + 2(aquec+qtdRampas);
          etapa = ETAPA_CONF_TEMP_FERV;
          menu = M_CONF_FERVURA;
        } else {
          etapa++;
        }
      } else {
        int x;
        if (btPress == PIN_BT_ADD) {
          x = 1;
        } else if (btPress == PIN_BT_SUB) {
          x = -1;
        }
        if (etapa == ETAPA_CONF_TEMP_PREAQC_BRASS) {
          brassagem.tempPreAquec = brassagem.tempPreAquec + x;
          updateConfBrassagemPreAquec(brassagem.tempPreAquec);
        } else if (etapa == ETAPA_CONF_QTD_RAMPAS) {
          brassagem.qtdRampas = brassagem.qtdRampas + x;
          updateConfBrassagemQtdRampas(brassagem.qtdRampas);
        } else {
          //definicao da posicao na matriz [y][ps]
          float y = etapa / 2.0 - 1;
          int ps = 0; if (x % 1 > 0)ps = 1;
          brassagem.rampas[(int)y][ps] = brassagem.rampas[(int)y][ps] + x;
          //y para 'n' da rampa; ps=0 para temperatura, ps=1 para tempo; valor;
          updateConfBrassagemRampas((int)y, ps, brassagem.rampas[(int)y][ps]);
        }
      }
      break;

    /*=============================================*/
    /*=============menu conf. fervura==============*/
    case M_CONF_FERVURA:
      if (btPress == PIN_BT_ENTER) {
        if (etapa == (fervura.qtdLupulo + 3 )) { //quantidade de lupulos + 3(temp+tempo+qtd);
          etapa = ETAPA_PREAQUEC;
          if (menuSelectTmp == M_CONF_BRASSAGEM) { //se selecionou no menu conf.Brassagem vai para etapa de brass.
            menu = M_BRASSAGEM;
          } else { // se selecionou no menu conf.fervura vai direto para ferv.
            menu = M_FERVURA;
          }
        } else {
          etapa++;
        }
      } else {
        int x;
        if (btPress == PIN_BT_ADD) {
          x = 1;
        } else if (btPress == PIN_BT_SUB) {
          x = -1;
        }
        if (etapa == ETAPA_CONF_TEMP_FERV) {
          fervura.tempFervura += fervura.tempFervura + x;
          updateConfFervuraTemp(fervura.tempFervura);
        } else if (etapa == ETAPA_CONF_DURC_FERV) {
          fervura.duracaoMin = fervura.duracaoMin + x;
          updateConfFervuraDuration(fervura.duracaoMin);
        } else if (etapa == ETAPA_CONF_QTD_LUPULO) {
          fervura.qtdLupulo = fervura.qtdLupulo + x;
          updateConfFervuraQtdLupulo(fervura.qtdLupulo);
        } else {
          fervura.lupulo[etapa - 2] = fervura.lupulo[etapa - 2] + x;
          updateConfFervuraLupulo(etapa - 2, fervura.lupulo[etapa - 2]);
        }
      }
      break;
    /*=============================================*/
    /*=============proc. brassagem=================*/
    case M_BRASSAGEM:
      switch (btPress) {
        case PIN_BT_ENTER:
          if(etapa==ETAPA_WAIT_CONFIRM_FERV){
            menu=M_FERVURA;
            etapa=ETAPA_PREAQUEC;
          }
          break;
        case PIN_BT_ADD:
          break;
        case PIN_BT_SUB:
          break;
      }
      /*=============================================*/
      /*=============proc. fervura===================*/
      break;
    case M_FERVURA:
      switch (btPress) {
        case PIN_BT_ENTER:
          if(etapa==ETAPA_WAIT_CONFIRM_END){
            menu=M_PRINCIPAL;
            etapa=0;
          }
          break;
        case PIN_BT_ADD:
          break;
        case PIN_BT_SUB:
          break;
      }
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

