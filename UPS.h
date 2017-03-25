//Delay de cada fim de ciclo do loop()
const int DELAY = 100;

int ups = 0;//controla o ups (update por segundo) de atualização da tela.
int sec = 0;//controle de segundos.
// Variavel para controle de tempo UPS;
unsigned long time;

void refreshUPS();
void startLoop();
void endLoop();

void startLoop(){
  time = millis();
}

void endLoop(){
  refreshUPS();
}

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

