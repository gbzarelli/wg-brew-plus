#include <LiquidCrystal.h>

//LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Simbolo de temperatura
uint8_t degree[8]  = {140,146,146,140,128,128,128,128};

// Metodos
void setupDisplay();
void updateMenuPrincipal(int index);

void updateConfBrassagemPreAquec(int temp);
void updateConfBrassagemQtdRampas(int qtd);
void updateConfBrassagemRampas(int pos,int tipo, int valor);

void updateConfFervuraQtdLupulo(int qtd);
void updateConfFervuraLupulo(int pos, int valor);
void updateConfFervuraDuration(int qtd);
void updateConfFervuraTemp(int temp);

void updatePreAquecBrassagem(int temp, int tempDesired);
void printTemp(int temp);
void updateWaitConfirmFerv();
void updateRampa(boolean start, int rampa,int maxRampa, int currentTemp,int tempRampa, int timeSec);
void updatePreAquecFervura(int temp, int tempDesired);
void updateWaitConfirmEnd(String msg);
void updateFervura(int currentTemp,int tempFervura,int lupulo, int maxLupulo, int timeSec);

void updateConfRefrQtdRampas(int qtd);
void updateConfRefrRampas(int pos,int tipo, int valor);
void updateRefrRampa(boolean start, int rampa,int maxRampa, int currentTemp,int tempRampa, int timeHoras, int horasRampa);

void setupDisplay(){
  // Inicializa LCD
  lcd.begin(16, 2);
  lcd.createChar(0, degree);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WG BREW PLUS V1");
  lcd.setCursor(0, 1);
  lcd.print("CARREGANDO...");
}
/**
 * Update de tela. Menu principal, escolha de brassagem ou fervura.
 */
void updateMenuPrincipal(int index){
  lcd.clear();
  
  lcd.setCursor(4, 0);
  if(index<=1){
    lcd.print("BRASSAGEM");
    lcd.setCursor(4, 1);
    lcd.print("FERVURA");
  }else{
    lcd.print("REFRIGERAR");
  }

  if(index==1){
    lcd.setCursor(0,1);
  }else{
    lcd.setCursor(0,0);
  }
  lcd.print(" -> ");
}

/**
 * Exibe temperatura de pre aquecimento da brassagem.
 */
void updateConfBrassagemPreAquec(int temp){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PRE AQUECIMENTO:");
  lcd.setCursor(0, 1);
  printTemp(temp);
}

/**
 * Exibe quantidade de rampas da brassagem
 */
void updateConfBrassagemQtdRampas(int qtd){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("QTD. RAMPAS:");
  lcd.setCursor(0, 1);
  lcd.print(qtd);
}

/**
 * Exibe informações de configuração das rampas da brassagem.
 * 
 * int pos = indice da configuracao da rampa.
 * int tipo = 0 para temperatura; 1 para tempo;
 * int valor = temperatura ou tempo;
 */
void updateConfBrassagemRampas(int pos,int tipo, int valor){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONF. RAMPA "+String(pos+1));
  lcd.setCursor(0, 1);
  if(tipo==0){
    lcd.print("TEMPERATURA: "+String(valor));
  }else{
    lcd.print("TEMPO(MIN): "+String(valor));
  }
}


/**
 * Exibe temperatura fervura
 */
void updateConfFervuraTemp(int temp){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TEMP P/I FERVURA:");
  lcd.setCursor(0, 1);
  printTemp(temp);
}
/**
 * Exibe duracao de fervura
 */
void updateConfFervuraDuration(int qtd){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DURACAO DA FERVURA:");
  lcd.setCursor(0, 1);
  lcd.print(qtd);
}
/**
 * Exibe quantidade de lupulos da fervura
 */
void updateConfFervuraQtdLupulo(int qtd){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("QTD. LUPULOS:");
  lcd.setCursor(0, 1);
  lcd.print(qtd);
}

/**
 * Configura o alarme do lupulo
 * 
 * int pos = indice da configuracao do lupulo.
 * int valor = tempo;
 */
void updateConfFervuraLupulo(int pos, int valor){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONF. LUPULO "+String(pos+1));
  lcd.setCursor(0, 1);
  lcd.print("NO MINUTO: "+String(valor));
}

/**
 * Atualiza a temperatura de pre aquecimento da brassagem
 * e exibe a temperatura desejada.
 */
void updatePreAquecBrassagem(int temp, int tempDesired){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PRE AQUECIMENTO");
  lcd.setCursor(0, 1);
  lcd.print("A:");
  printTemp(temp);
  lcd.print(" D:");
  printTemp(tempDesired);  
}

/**
 * Aguarda pressionar o botao para iniciar a fervura;
 */
void updateWaitConfirmFerv(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INICIAR FERVURA");
  lcd.setCursor(0, 1);
  lcd.print("PRESS. ENTER");
}

/**
     -0123456789012345-
     |R:10/10  T:10ºC |
     |10:39    A:20ºC |
    
     -0123456789012345-
     |R:2/5    T:10ºC |
     |Wait...  A:20ºC |
 */
 
void updateRampa(boolean start, int rampa,int maxRampa, int currentTemp,int tempRampa, int timeSec){
  lcd.clear();
  lcd.setCursor(0, 0);
  String l1 = "R:" + String(rampa) + "/" + maxRampa;
  lcd.print(l1);
  lcd.setCursor(9, 0);
  lcd.print("T:");
  printTemp(tempRampa);
  lcd.setCursor(0, 1);
  String l2;
  if(start){
    int m=timeSec/60;
    int s=timeSec%60;
    l2 = (m<10?"0"+String(m):m) + ":" + (s<10?"0"+String(s):s);
  }else{
    l2 = "WAIT...";
  }
  lcd.print(l2);
  lcd.setCursor(9, 1);
  lcd.print("A:");
  printTemp(currentTemp);
}

void updatePreAquecFervura(int temp, int tempDesired){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FERVENDO...");
  lcd.setCursor(0, 1);
  lcd.print("A:");
  printTemp(temp);
  lcd.print(" D:");
  printTemp(tempDesired);  
}

void updateWaitConfirmEnd(String msg){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg);
  lcd.setCursor(0, 1);
  lcd.print("PRESS. ENTER");
}

/**
     -0123456789012345-
     |L:10/10  T:10ºC |
     |10:39    A:20ºC |
*/
void updateFervura(int currentTemp,int tempFervura,int lupulo, int maxLupulo, int timeSec){
  lcd.clear();
  lcd.setCursor(0, 0);
  String l1 = "L:" + String(lupulo) + "/" + maxLupulo;
  lcd.print(l1);
  lcd.setCursor(9, 0);
  lcd.print("T:");
  printTemp(tempFervura);
  lcd.setCursor(0, 1);
  String l2;
  int m=timeSec/60;
  int s=timeSec%60;
  l2 = (m<10?"0"+String(m):m) + ":" + (s<10?"0"+String(s):s);
  lcd.print(l2);
  lcd.setCursor(9, 1);
  lcd.print("A:");
  printTemp(currentTemp);
}



/**
 * Exibe quantidade de rampas da refrigeracao
 */
void updateConfRefriQtdRampas(int qtd){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("QTD. RAMPAS:");
  lcd.setCursor(0, 1);
  lcd.print(qtd);
}

/**
 * Exibe informações de configuração das rampas da refrigeracao.
 * 
 * int pos = indice da configuracao da rampa.
 * int tipo = 0 para temperatura; 1 para tempo;
 * int valor = temperatura ou tempo;
 */
void updateConfRefriRampas(int pos,int tipo, int valor){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONF. RAMPA "+String(pos+1));
  lcd.setCursor(0, 1);
  if(tipo==0){
    lcd.print("TEMPERATURA: "+String(valor));
  }else{
    lcd.print("HORAS: "+String(valor));
  }
}
/**
     -0123456789012345-
     |R:10/10  T:10ºC |
     |00/90h   A:20ºC |
    
     -0123456789012345-
     |R:2/5    T:10ºC |
     |Wait...  A:20ºC |
 */
 
void updateRefriRampa(boolean start, int rampa,int maxRampa, int currentTemp,int tempRampa, int timeHoras, int horasRampa){
  lcd.clear();
  lcd.setCursor(0, 0);
  String l1 = "R:" + String(rampa) + "/" + maxRampa;
  lcd.print(l1);
  lcd.setCursor(9, 0);
  lcd.print("T:");
  printTemp(tempRampa);
  lcd.setCursor(0, 1);
  String l2;
  if(start){
    l2 = (timeHoras<10?"0"+String(timeHoras):timeHoras) + "/" + (horasRampa<10?"0"+String(horasRampa):horasRampa) + "h";
  }else{
    l2 = "WAIT...";
  }
  lcd.print(l2);
  lcd.setCursor(9, 1);
  lcd.print("A:");
  printTemp(currentTemp);
}


/**
 * Mostra a temperatura passada com o simbolo ex: '20ºC'
 */
void printTemp(int temp){
    lcd.print(temp);
  #if ARDUINO >= 100
    lcd.write((byte)0);
  #else
    lcd.print(0, BYTE);
  #endif
    lcd.print("C");
}

