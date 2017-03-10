//temperatura de chaveamento em Cº
const int SWITCH_TEMP_VARIABLE = 2;

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

int vccPin = 3;
int gndPin = 2;

int PIN_RESISTENCE = 17;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

void setupSensors();
void refreshResistence(int tempDesired);
double getThermoC();

void setupSensors() {
  // Configuração de pinos arduino
  pinMode(vccPin, OUTPUT); digitalWrite(vccPin, HIGH);
  pinMode(gndPin, OUTPUT); digitalWrite(gndPin, LOW);
  pinMode(PIN_RESISTENCE, OUTPUT);
}

/**
    Verifica se a temperatura do sensor esta adequada a desejada e
    realiza o chaveamento necessario para adequar.
*/
void refreshResistence(int tempDesired) {
  tempDesired = getThermoC() - tempDesired;
  if (tempDesired < (SWITCH_TEMP_VARIABLE * -1)) {
    digitalWrite(PIN_RESISTENCE, HIGH);
  } else if (tempDesired > SWITCH_TEMP_VARIABLE) {
    digitalWrite(PIN_RESISTENCE, LOW);
  }
}

/**
   Retorno da temperatura do sensor.
*/
double getThermoC() {
  return thermocouple.readCelsius();
}

