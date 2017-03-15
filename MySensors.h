//temperatura de chaveamento em Cº
const int SWITCH_TEMP_VARIABLE = 2;

const int PIN_THERMO_DO=4;
const int PIN_THERMO_CS=5;
const int PIN_THERMO_CLK=6;

const int PIN_RESISTENCE = 17;

//int vccPin = 3;int gndPin = 2;

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
  turnOffResistence();
}

/**
    Verifica se a temperatura do sensor esta adequada a desejada e
    realiza o chaveamento necessario para adequar.
*/
void refreshResistence(int tempDesired) {
  tempDesired = getThermoC() - tempDesired;
  if (tempDesired < (SWITCH_TEMP_VARIABLE * -1)) {
    turnOnResistence();
  } else if (tempDesired > SWITCH_TEMP_VARIABLE) {
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
  return thermocouple.readCelsius();
}

