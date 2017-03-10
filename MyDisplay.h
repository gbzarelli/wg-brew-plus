#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

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
void updateConfFervuraDuracao(int qtd);
void updateConfFervuraTemp(int temp);

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
  lcd.print("BRASSAGEM");
  lcd.setCursor(4, 1);
  lcd.print("FERVURA");
  if(index==0){
    lcd.setCursor(0,0);
  }else{
    lcd.setCursor(0,1);
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
  lcd.print(temp);
  #if ARDUINO >= 100
    lcd.write((byte)0);
  #else
    lcd.print(0, BYTE);
  #endif
    lcd.print("C");
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
  lcd.print("CONF. RAMPA "+(pos+1));
  lcd.setCursor(0, 1);
  if(tipo=0){
    lcd.print("TEMPERATURA: "+valor);
  }else{
    lcd.print("TEMPO(MIN): "+valor);
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
  lcd.print(temp);
  #if ARDUINO >= 100
    lcd.write((byte)0);
  #else
    lcd.print(0, BYTE);
  #endif
    lcd.print("C");
}
/**
 * Exibe duracao de fervura
 */
void updateConfFervuraDuracao(int qtd){
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
  lcd.print("CONF. LUPULO "+(pos+1));
  lcd.setCursor(0, 1);
  lcd.print("TEMPO(MIN): "+valor);
}

