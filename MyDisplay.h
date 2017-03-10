#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// Simbolo de temperatura
uint8_t degree[8]  = {140,146,146,140,128,128,128,128};

// Metodos
void setupDisplay();
void updateMenuPrincipal(int index);

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
