#include <Wire.h>
#include "MyDisplay.h"
#include "MySensors.h"
#include "UPS.h"
#include "MyButtons.h"

//Constantes que define a tela atual a ser exibida.
const int M_PRINCIPAL = 0;

const int M_CONF_BRASSAGEM = 1;
const int M_CONF_FERVURA = 2;
const int M_CONF_REFRIGERAR = 5;

const int M_BRASSAGEM = 3;
const int M_FERVURA = 4;
const int M_REFRIGERAR = 6;

//Constantes utilizadas na configuração da brassagem e fervura
const int ETAPA_CONF_TEMP_FERV = 0;
const int ETAPA_CONF_DURC_FERV = 1;
const int ETAPA_CONF_QTD_LUPULO = 2;
//--
const int ETAPA_CONF_TEMP_PREAQC_BRASS = 0;
const int ETAPA_CONF_QTD_RAMPAS = 1;
//--


//Constantes para processo de brassagem e fervura (execucao)
const int ETAPA_PREAQUEC=0;
const int ETAPA_RAMPAS=1;
const int ETAPA_WAIT_CONFIRM_FERV=2;
const int ETAPA_FERVER=3;
const int ETAPA_FERVURA_LUP=4;
const int ETAPA_WAIT_CONFIRM_END=5;

// Constanstes de configuracao de tempo;
const int SEC_ALARM_LUPULO = 10;//duração do alarme do lupulo
const int SEC_ALARM_RAMPA = 10;//duração do alarme de rampa
const int SEC_ALARM_START_COUNT = 2;//duração do alarme quando inicia o cronometro
const int SEC_ALARM_WAIT_ENTER = 30;//duração do alarme AGUARDANDO PROSSEGUIR


//Variaveis de controle
int menu = 0;
int etapa = 0;
int indexTmp = 0;//indice temporario.(utilizado em qualquer rotina)
int menuSelectTmp;//controle de qual item do menu foi selecionado.
boolean startTimer;//controle para definir o inicio do timer;

//*************************************************************
// Estrutura de configuração das etapas de brassagem, fervura e refrigerar
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

struct config_refrigerar {
  int qtdRampas;
  int rampas[10][2];//[0,0]=temp;[0,1]=durac (horas)
};

struct config_fervura fervura;
struct config_brassagem brassagem;
struct config_refrigerar refrigerar;

//*************************************************************

// Metodos
void processRefrigerar();
void processBtPress(int btPress);
void processBrassagem();
void processFervura();
void zerarDados();

void setup() {
  Serial.begin(9600);
  setupDisplay();
  setupSensors();
  setupBuzzer();
  setupButtons();
  
  zerarDados();
  updateMenuPrincipal(0);
}

void zerarDados(){
  indexTmp=0;
  menu=M_PRINCIPAL;
  etapa=0;
  brassagem.tempPreAquec = 50;
  brassagem.qtdRampas = 5;
  for (int i = 0 ; i < 10; i++) {
    for (int b = 0; b < 2; b++) {
      brassagem.rampas[i][b] = 0;
    }
  }

  refrigerar.qtdRampas=1;
  for (int i = 0 ; i < 10; i++) {
    for (int b = 0; b < 2; b++) {
      refrigerar.rampas[i][b] = 0;
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
  startLoop();
  
  loopBuzzer();
  loopSensors();

  int btPressed = getBtPress();
  
  if (btPressed != -1) {
    processBtPress(btPressed);
  }

  if (M_REFRIGERAR == menu) {
    processRefrigerar();
  } else  if (M_BRASSAGEM == menu) {
    processBrassagem();
  } else if (M_FERVURA == menu) {
    processFervura();
  }

  sec = endLoop();
}

void processRefrigerar(){
  int temperature;
  int duration;
  
  switch(etapa){
    //**********************************
    case ETAPA_RAMPAS:
      temperature = refrigerar.rampas[indexTmp][0];
      duration = refrigerar.rampas[indexTmp][1] * 3600;
     
      //se o chronometro nao foi iniciado e a temperatura chegou no nivel da rampa,
      //inicia o chronometro;
      if(!startTimer && inTemperature(temperature)) {
        alarmAsync(SEC_ALARM_RAMPA);
        startTimer=true;
        sec=0;
      }
      
      if(startTimer && sec >= duration ){
        //Verifica se ainda existe rampas configuradas:
        if(indexTmp+1 < refrigerar.qtdRampas){
          alarmAsync(SEC_ALARM_START_COUNT);
          indexTmp++;//Se existir incrementa;
        }else{
          indexTmp=0;
          etapa = ETAPA_WAIT_CONFIRM_END;
        }
        startTimer=false;//para chrono para atingir temperatura da prox rampa;
        break;
      }

      if(ups>=1000)updateRefriRampa(startTimer, (indexTmp+1), refrigerar.qtdRampas,getThermoC(), temperature, (sec/3600), refrigerar.rampas[indexTmp][1]);
      refreshResistenceRefri(temperature);
      
      break;

    //************************************************************
    //******AGUARDANDO CONFIRMACAO PARA FIM**********
    case ETAPA_WAIT_CONFIRM_END:
        if(0 == indexTmp){
          indexTmp++;
          alarmAsync(SEC_ALARM_WAIT_ENTER);
          turnOffResistence();//DESLIGA RESISTENCIA
          updateWaitConfirmEnd("FIM DO REFRIGERAR");
        }
      break;
  }

}

void processBrassagem() {
  int temperature;
  int duration;
  
  switch(etapa){
    //**********************************
    case ETAPA_PREAQUEC:
      temperature = brassagem.tempPreAquec;
      if(inTemperature(temperature)){
        startTimer=false;
        etapa=ETAPA_RAMPAS;
        indexTmp=0;
      }
      if(ups>=1000)updatePreAquecBrassagem((int)getThermoC(),brassagem.tempPreAquec);
    break;

    //**********************************
    case ETAPA_RAMPAS:
      temperature = brassagem.rampas[indexTmp][0];
      duration = brassagem.rampas[indexTmp][1] * 60;
     
      //se o chronometro nao foi iniciado e a temperatura chegou no nivel da rampa,
      //inicia o chronometro;
      if(!startTimer && inTemperature(temperature)) {
        alarmAsync(SEC_ALARM_RAMPA);
        startTimer=true;
        sec=0;
      }
      
      if(startTimer && sec >= duration){
        //Verifica se ainda existe rampas configuradas:
        if(indexTmp+1 < brassagem.qtdRampas){
          alarmAsync(SEC_ALARM_START_COUNT);
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

      if(ups>=1000)updateRampa(startTimer, (indexTmp+1), brassagem.qtdRampas,getThermoC(), temperature, duration);      
      break;

    //************************************************************
    //******AGUARDANDO CONFIRMACAO PARA INICIO DE FERCURA**********
    case ETAPA_WAIT_CONFIRM_FERV:
      if(0 == indexTmp){
        indexTmp++;
        alarmAsync(SEC_ALARM_WAIT_ENTER);
        turnOffResistencePID();//DESLIGA RESISTENCIA
        updateWaitConfirmFerv();
      }
    return;
  }
  refreshResistencePID(temperature);

}

void processFervura() {
  
    switch(etapa){
      //**********************************
      case ETAPA_PREAQUEC:
        if(inTemperature(fervura.tempFervura)){
          alarmAsync(SEC_ALARM_START_COUNT);
          sec=0;
          indexTmp=0;
          etapa=ETAPA_FERVURA_LUP;
        }
        if(ups>=1000)updatePreAquecFervura((int)getThermoC(),fervura.tempFervura);
      break;
      
      //**********************************
      case ETAPA_FERVURA_LUP:
        //Fim do tempo de fervura!
        if(sec >= (fervura.duracaoMin*60)){
          indexTmp=0;
          etapa=ETAPA_WAIT_CONFIRM_END;
          break;
        }
        
        if(sec > (fervura.lupulo[indexTmp] * 60) && indexTmp < fervura.qtdLupulo){
            alarmAsync(SEC_ALARM_LUPULO);
            indexTmp++;
        }
        if(ups>=1000)updateFervura((int)getThermoC(),fervura.tempFervura,indexTmp,fervura.qtdLupulo,sec);
      break;
      //************************************************************
      //******AGUARDANDO CONFIRMACAO PARA TERMINAR**********
      case ETAPA_WAIT_CONFIRM_END:
        if(0 == indexTmp){
          indexTmp++;
          alarmAsync(SEC_ALARM_WAIT_ENTER);
          turnOffResistencePID();//DESLIGA RESISTENCIA
          updateWaitConfirmEnd("FIM DA FERVURA");
        }
      return;
    }
    refreshResistencePID(fervura.tempFervura);
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
//mudanca de etapa refresh screen; processBtPress(-1);
void processBtPress(int btPress) {
  switch (menu) {
    /*=============================================*/
    /*=============menu principal==================*/
    case M_PRINCIPAL:
      switch (btPress) {
        case PIN_BT_ENTER:
          if (0 == indexTmp) {
            etapa = ETAPA_CONF_TEMP_PREAQC_BRASS;
            menu = M_CONF_BRASSAGEM;
          } else if (1 == indexTmp) {
            etapa = ETAPA_CONF_TEMP_FERV;
            menu = M_CONF_FERVURA;
          } else if (2 == indexTmp) {
          	etapa = ETAPA_CONF_QTD_RAMPAS;
            menu = M_CONF_REFRIGERAR;
          }
          menuSelectTmp = menu;
          processBtPress(-1);
          return;

        case PIN_BT_ADD:
          if(indexTmp>0){
          	indexTmp--;	
          }
          break;

        case PIN_BT_SUB:
          if(indexTmp<2){
          	indexTmp++;
          }
          break;
      }
      updateMenuPrincipal(indexTmp);
      break;
     /*=============================================*/
    /*=============menu conf. refrigerar============*/
    case M_CONF_REFRIGERAR:
     if (PIN_BT_ENTER==btPress) {
        if (etapa == (refrigerar.qtdRampas * 2)+1) { //quantidade de rampas*2(temp+time) + 0(qtdRampas);
          //Passou da ultima etapa, iniciar processo:
        startTimer=false;
        etapa=ETAPA_RAMPAS;
        indexTmp=0;
          menu = M_REFRIGERAR;
        } else {
          etapa++;
        }
        processBtPress(-1);
      } else {
        int x;
        if (PIN_BT_ADD==btPress) {
          x = 1;
        } else if (PIN_BT_SUB==btPress) {
          x = -1;
        }
              
        if (ETAPA_CONF_QTD_RAMPAS==etapa) {
          if(refrigerar.qtdRampas == 1 && x == -1){
            break;
          }
          refrigerar.qtdRampas += x;
          updateConfRefriQtdRampas(refrigerar.qtdRampas);
        } else {
          //definicao da posicao na matriz [y][ps]
          float y = etapa / 2.0 - 1;
          int ps = 0; 
          if (fmod(y,1) > 0.000){
            ps = 1;
          }
          if(ps==1 && refrigerar.rampas[(int)y][ps] == 0 && x == -1){
            break;
          }
          refrigerar.rampas[(int)y][ps] += x;//Define o valor na rampa
          if((int)y < refrigerar.qtdRampas-1){
            refrigerar.rampas[((int)y)+1][ps] = refrigerar.rampas[(int)y][ps];//Define o mesmo valor para proxima rampa afim de ajudar na definicao.
          }
          //y para 'n' da rampa; ps=0 para temperatura, ps=1 para tempo; valor;
          updateConfRefriRampas((int)y, ps, refrigerar.rampas[(int)y][ps]);
        }
      }
      break;
    /*=============================================*/
    /*=============menu conf. brassagem============*/
    case M_CONF_BRASSAGEM:
      if (PIN_BT_ENTER==btPress) {
        if (etapa == (brassagem.qtdRampas * 2 + 1 )) { //quantidade de rampas*2(temp+time) + 1(0-aquec+1-qtdRampas);
          etapa = ETAPA_CONF_TEMP_FERV;
          menu = M_CONF_FERVURA;
        } else {
          etapa++;
        }
        processBtPress(-1);
      } else {
        int x;
        if (PIN_BT_ADD==btPress) {
          x = 1;
        } else if (PIN_BT_SUB==btPress) {
          x = -1;
        }
              
        if (ETAPA_CONF_TEMP_PREAQC_BRASS==etapa) {
          if(brassagem.tempPreAquec == 0 && x == -1){
            break;
          }
          brassagem.tempPreAquec += x;
          updateConfBrassagemPreAquec(brassagem.tempPreAquec);
        } else if (ETAPA_CONF_QTD_RAMPAS==etapa) {
          if(brassagem.qtdRampas == 1 && x == -1){
            break;
          }
          brassagem.qtdRampas += x;
          updateConfBrassagemQtdRampas(brassagem.qtdRampas);
        } else {
          //definicao da posicao na matriz [y][ps]
          float y = etapa / 2.0 - 1;
          int ps = 0; 
          if (fmod(y,1) > 0.000){
            ps = 1;
          }
          if(brassagem.rampas[(int)y][ps] == 0 && x == -1){
            break;
          }
          brassagem.rampas[(int)y][ps] += x;//Define o valor na rampa
          if((int)y < brassagem.qtdRampas-1){
            brassagem.rampas[((int)y)+1][ps] = brassagem.rampas[(int)y][ps];//Define o mesmo valor para proxima rampa afim de ajudar na definicao.
          }
          //y para 'n' da rampa; ps=0 para temperatura, ps=1 para tempo; valor;
          updateConfBrassagemRampas((int)y, ps, brassagem.rampas[(int)y][ps]);
        }
      }
      break;

    /*=============================================*/
    /*=============menu conf. fervura==============*/
    case M_CONF_FERVURA:
      if (PIN_BT_ENTER==btPress) {
        if (etapa == (fervura.qtdLupulo + 2 )) { //quantidade de lupulos + 2(0temp+1tempo+2qtd);
          etapa = ETAPA_PREAQUEC;
          if (M_CONF_BRASSAGEM==menuSelectTmp) { //se selecionou no menu conf.Brassagem vai para etapa de brass.
            menu = M_BRASSAGEM;
          } else { // se selecionou no menu conf.fervura vai direto para ferv.
            menu = M_FERVURA;
          }
        } else {
          etapa++;
        }
        processBtPress(-1);
      } else {
        int x;
        if (PIN_BT_ADD==btPress) {
          x = 1;
        } else if (PIN_BT_SUB==btPress) {
          x = -1;
        }
        if (ETAPA_CONF_TEMP_FERV==etapa) {
          if(fervura.tempFervura == 0 && x == -1){
            break;
          }
          fervura.tempFervura += x;
          updateConfFervuraTemp(fervura.tempFervura);
          
        } else if (etapa == ETAPA_CONF_DURC_FERV) {
          if(fervura.duracaoMin == 0 && x == -1){
            break;
          }          
          fervura.duracaoMin += x;
          updateConfFervuraDuration(fervura.duracaoMin);
        } else if (etapa == ETAPA_CONF_QTD_LUPULO) {
          if(fervura.qtdLupulo == 0 && x == -1){
            break;
          }
          fervura.qtdLupulo = fervura.qtdLupulo + x;
          updateConfFervuraQtdLupulo(fervura.qtdLupulo);
        } else {
          int indxLup = etapa-3;
          if(fervura.lupulo[indxLup] == 0 && x == -1){
            break;
          }

          if(indxLup > 0 && fervura.lupulo[indxLup] == fervura.lupulo[indxLup-1]+1 && -1 == x){
            break;
          }
          
          fervura.lupulo[indxLup] += x;//Define o valor do lupulo.
          if(indxLup < fervura.qtdLupulo-1){
            fervura.lupulo[indxLup+1] = fervura.lupulo[indxLup]+1;//Define o mesmo valor no proximo lupulo pois o tempo nao pode ser menor que o anterior.
          }
          updateConfFervuraLupulo(indxLup, fervura.lupulo[indxLup]);
        }
      }
      break;
    /*=============================================*/
    /*=============proc. brassagem=================*/
    case M_BRASSAGEM:
      switch (btPress) {
        case PIN_BT_ENTER:
          if(ETAPA_WAIT_CONFIRM_FERV==etapa){
            indexTmp=0;
            menu=M_FERVURA;
            etapa=ETAPA_PREAQUEC;
          }
          stopAlarm();
          processBtPress(-1);
          break;
        case PIN_BT_ADD:
        case PIN_BT_SUB:
	        int x = 0;
	        if(PIN_BT_ADD==btPress){
	        	x = 1;
	        }else{
	        	x = -1;
	        }

  		    if(ETAPA_PREAQUEC==etapa){
  	      		brassagem.tempPreAquec += x;
  	    	}else if(ETAPA_RAMPAS==etapa){
  	  	  		brassagem.rampas[indexTmp][0] += x;
  			  }
        break;
      }
      break;    	
      /*=============================================*/
      /*=============proc. fervura===================*/
    case M_FERVURA:
      switch (btPress) {
        case PIN_BT_ENTER:
          if(ETAPA_WAIT_CONFIRM_END==etapa){
            zerarDados();
          }
          stopAlarm();
          processBtPress(-1);
          break;
        case PIN_BT_ADD:
        case PIN_BT_SUB:
    			int x = 0;
    			if(PIN_BT_ADD==btPress){
    				x = 1;
    			}else{
    				x = -1;
    			}

          if(ETAPA_PREAQUEC==etapa || ETAPA_FERVURA_LUP==etapa){
          	fervura.tempFervura += x;
          }
          break;
      }
      break;
      /*=============================================*/
      /*=============proc. fervura===================*/
    case M_REFRIGERAR:
      switch (btPress) {
        case PIN_BT_ENTER:
          if(ETAPA_WAIT_CONFIRM_END==etapa){
            zerarDados();
          }
          stopAlarm();
          processBtPress(-1);
          break;
        case PIN_BT_ADD:
        case PIN_BT_SUB:
          int x = 0;
          if(PIN_BT_ADD==btPress){
            x = 1;
          }else{
            x = -1;
          }

          if(ETAPA_RAMPAS==etapa){
              refrigerar.rampas[indexTmp][0] += x;
          }
          break;
      }
      break;
  }
}

