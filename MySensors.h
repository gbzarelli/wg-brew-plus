#include <max6675.h>

const boolean DEBUG = false;

//temperatura de chaveamento em Cº
const int SWITCH_TEMP_VARIABLE = 2;

const int PIN_THERMO_DO=A3;
const int PIN_THERMO_CS=A4;
const int PIN_THERMO_CLK=A5;

const int PIN_RESISTENCE = A2;

MAX6675 thermocouple(PIN_THERMO_CLK, PIN_THERMO_CS, PIN_THERMO_DO);

void setupSensors();
void refreshResistence(int tempDesired);
double getThermoC();
void turnOnResistence();
void turnOffResistence();

void setupSensors() {
  // Configuração de pinos arduino
  //pinMode(vccPin, OUTPUT); digitalWrite(vccPin, HIGH);pinMode(gndPin, OUTPUT); digitalWrite(gndPin, LOW);
  pinMode(PIN_RESISTENCE, OUTPUT);
  
  if(DEBUG){
    pinMode(PIN_THERMO_DO,OUTPUT);
    pinMode(PIN_THERMO_CS,INPUT);
    pinMode(PIN_THERMO_CLK,OUTPUT);
    digitalWrite(PIN_THERMO_DO,HIGH);
    digitalWrite(PIN_THERMO_CLK,LOW);
  }else{
    //TODO
  }
  
  // Aguarda estabilização.
  delay(5000);
  
  turnOffResistence();
}

/**
    Verifica se a temperatura do sensor esta adequada a desejada e
    realiza o chaveamento necessario para adequar.
*/
void refreshResistence(int tempDesired) {
  tempDesired = getThermoC() - tempDesired;
  if (tempDesired <= (SWITCH_TEMP_VARIABLE * -1)) {
    turnOnResistence();
  } else if (tempDesired >= SWITCH_TEMP_VARIABLE) {
    turnOffResistence();
  }
}

/**
 * Liga a resistencia
 */
void turnOnResistence(){
  digitalWrite(PIN_RESISTENCE, HIGH);
}

/**
 * Desliga a resistencia
 */
void turnOffResistence(){
  digitalWrite(PIN_RESISTENCE, LOW);
}
/**
   Retorno da temperatura do sensor.
*/
double getThermoC() {
  if(DEBUG){
    return map(analogRead(PIN_THERMO_CS),0,1023,0,200);
  }else{
   Serial.print("C = "); 
   Serial.println(thermocouple.readCelsius());
   Serial.print("F = ");
   Serial.println(thermocouple.readFahrenheit());
   
    return thermocouple.readCelsius();
  }
}

