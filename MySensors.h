#include <max6675.h>
#include <PID_v1.h>

const int SENSOR_MAX6675=1;
const int SENSOR_LM35=2;
const int SENSOR_DEBUG=3;

const int TYPE_SENSOR = SENSOR_LM35;

//temperatura de chaveamento em Cº
const int SWITCH_TEMP_VARIABLE_NEGATIVE = 1;//temperatura de troca para ligar/desligar para temperatura negativa;
const int SWITCH_TEMP_VARIABLE_VALIDATION = 2;//range de temperatura utilizado na validação.

const int PIN_LM35_0 = A0;
const int PIN_LM35_1 = A1;
const int PIN_RESISTENCE = A2;
const int PIN_THERMO_DO=A3;
const int PIN_THERMO_CS=A4;
const int PIN_THERMO_CLK=A5;

MAX6675 thermocouple(PIN_THERMO_CLK, PIN_THERMO_CS, PIN_THERMO_DO);

void setupSensors();
void loopSensors();
void refreshResistence(int tempDesired);
void refreshResistencePID(int tempDesired);
void refreshResistenceRefri(int tempDesired);
double getThermoC();
void turnOnResistence();
void turnOffResistence();
double getThermoLM35();
double getThremoMax6675();
void controllPID();
boolean inTemperature(int temperature);

void setupSensors() {
  // Configuração de pinos arduino
  pinMode(PIN_RESISTENCE, OUTPUT);
  pinMode(PIN_LM35_0,INPUT);pinMode(PIN_LM35_1,INPUT);

  if(TYPE_SENSOR == SENSOR_DEBUG){
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

//*************************************************************************************
//**PID CONTROLL************************************************************************
//*************************************************************************************
boolean startPID=false;

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=20, Ki=0, Kd=0;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
long myTimePID=0;
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

void loopSensors(){
  if(startPID && (millis()-myTimePID)>1000){
      myTimePID=millis();
      controllPID();
  }
}

void controllPID(){
  Input = getThermoC();
  myPID.Compute();
  Serial.print("Output: ");
  Serial.println(Output);
  if (Output > 70){
    turnOnResistence();
  }else{
    turnOffResistence();
  }
  
}

/**
    Verifica se a temperatura do sensor esta adequada a desejada e
    realiza o chaveamento necessario para adequar utilizando controle PID.
    Método async.
*/
void refreshResistencePID(int tempDesired){
  Setpoint = tempDesired;
  if(!startPID){
    Input = getThermoC();
    myPID.SetMode(AUTOMATIC);
    startPID=true;
  }
}

/**
  Para com o processo de controle de temperatura PID
*/
void turnOffResistencePID(){
  turnOffResistence();
  startPID=false;
  Setpoint=0;
}

/**
    Verifica se a temperatura do sensor esta adequada a desejada e
    realiza o chaveamento necessario para adequar.
*/
void refreshResistence(int tempDesired) {
  tempDesired = getThermoC() - tempDesired;
  if (tempDesired <= 0) {
    turnOnResistence();
  } else{
    turnOffResistence();
  }
}

/**
    Verifica se a temperatura do sensor esta adequada a desejada e
    realiza o chaveamento necessario para adequar.
*/
void refreshResistenceRefri(int tempDesired) {
  tempDesired = getThermoC() - (tempDesired - SWITCH_TEMP_VARIABLE_NEGATIVE);
  if (tempDesired > 0) {
    turnOnResistence();
  } else{
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
//40 -37
boolean inTemperature(int temperature){
  if(getThermoC() >= (temperature-SWITCH_TEMP_VARIABLE_VALIDATION) && getThermoC() <= (temperature + SWITCH_TEMP_VARIABLE_VALIDATION)){
    Serial.print("VALIDADO: ");Serial.print(temperature);Serial.print(" THERMO: ");Serial.println(getThermoC());
    return true;
  }
  return false;
}

/**
   Retorno da temperatura do sensor.
*/
double getThermoC() {
  if(TYPE_SENSOR==SENSOR_MAX6675){
    return getThremoMax6675();
  }else if(TYPE_SENSOR==SENSOR_LM35){
    return getThermoLM35();
  }else if(TYPE_SENSOR==SENSOR_DEBUG){
    return map(analogRead(PIN_THERMO_CS),0,1023,-20,200);
  }
  return -100;
}

/**
  Pega a temperatura do sensor MAX 6675
*/
double getThremoMax6675(){
  return thermocouple.readCelsius();
}

/**
  Pega a temperatura do sensor LM35
*/
double getThermoLM35(){
  double temp = analogRead(PIN_LM35_0) - analogRead(PIN_LM35_1);
  return (500*temp)/1023;
}

