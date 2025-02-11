#include <Wire.h>
#include <DFRobot_RGBLCD1602.h>
#include <EEPROM.h>
#include <TimerOne.h>

const int leds[] = {12, 11, 10, 9};
const int buttons[] = {8, 7, 6, 5};

int sequence[10];
int currentLength = 1;
int playerIndex = 0;
int score = 0;
int sessionHighScore = 0; // Pelaajan istunnon huipputulos

volatile int secondsElapsed = 0;
volatile bool timeOut = false;

// LCD:n käyttöönotto
DFRobot_RGBLCD1602 lcd(0x27, 16, 2);

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(leds[i], OUTPUT);
    pinMode(buttons[i], INPUT_PULLUP);
    digitalWrite(leds[i], LOW); // Varmistaa, että kaikki ledit ovat pois päältä alussa
  }

  lcd.init();
  int highScore = EEPROM.read(0);
  lcd.clear();
  lcd.print("Huipputulos: ");
  lcd.print(highScore);
  delay(3000);
  lcd.clear();

  startSequence();
  randomSeed(analogRead(A0)); // Alustetaan satunnaisluku

  Timer1.initialize(1000000); // Ajastin 1 sekunnin syklillä
  Timer1.attachInterrupt(timerISR);
  Timer1.stop();
}

void loop() {
  lcd.clear();
  lcd.print("Toista sarja!");
  delay(1000);

  // Näytä LED-sarja
  showSequence();

  // Käsittele pelaajan syötteet
  if (!handlePlayerInput()) {
    return; // Peli päättyy, jos pelaaja epäonnistuu
  }

  // Pisteen lisäys ja sarjan pidentäminen
  score++;
  sessionHighScore = max(sessionHighScore, score);
  lcd.clear();
  lcd.print("Pisteet: ");
  lcd.print(score);
  delay(2000);

  sequence[currentLength] = random(0, 4); // Lisää uusi LED sarjaan
  currentLength++;
}

// Funktio LED-sarjan näyttämiseen
void showSequence() {
  for (int i = 0; i < currentLength; i++) {
    digitalWrite(leds[sequence[i]], HIGH); // LED päälle
    delay(500);
    digitalWrite(leds[sequence[i]], LOW); // LED pois päältä
    delay(300); // Viive kaksoispainallusten estämiseksi
  }
}

// Funktio pelaajan syötteiden käsittelyyn
bool handlePlayerInput() {
  playerIndex = 0;
  secondsElapsed = 0;
  timeOut = false;
  Timer1.start();

  while (playerIndex < currentLength) {
    if (timeOut) {
      gameOver("Liian hidas!");
      return false;
    }

    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttons[i]) == LOW) { // Tarkista, painetaanko nappulaa
        if (i == sequence[playerIndex]) { // Oikea nappula painettu
          lcd.clear();
          lcd.print("Oikein!");
          digitalWrite(leds[i], HIGH);
          delay(500);
          digitalWrite(leds[i], LOW);
          playerIndex++;
          secondsElapsed = 0;
          timeOut = false;
          Timer1.restart();
        } else {
          gameOver("Vaarin!");
          return false;
        }
        delay(300); // Viive kaksoispainallusten estämiseksi
      }
    }
  }
  Timer1.stop();
  return true; // Pelaaja suoritti sarjan oikein
}

void startSequence() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    delay(300);
    digitalWrite(leds[i], LOW);
  }

  for (int j = 0; j < 2; j++) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(leds[i], HIGH);
    }
    delay(500);
    for (int i = 0; i < 4; i++) {
      digitalWrite(leds[i], LOW);
    }
    delay(500);
  }

  lcd.clear();
  lcd.print("Valmis!");
  delay(1000);
}

void gameOver(String reason) {
  Timer1.stop();
  lcd.clear();
  lcd.print(reason);
  delay(2000);

  int highScore = EEPROM.read(0);
  if (score > highScore) {
    EEPROM.write(0, score);
    lcd.clear();
    lcd.print("Uusi huipputulos!");
    delay(2000);
  }

  lcd.clear();
  lcd.print("Peli ohi!");
  lcd.setCursor(0, 0);
  lcd.print("Pisteet: ");
  lcd.print(score);
  lcd.setCursor(0, 1);
  lcd.print("Huipputulos: ");
  lcd.print(EEPROM.read(0));
  delay(4000);

  resetGame();
}

void resetGame() {
  lcd.clear();
  lcd.print("Aloita alusta!");
  currentLength = 1;
  score = 0;
  delay(2000);
}

void timerISR() {
  secondsElapsed++;
  if (secondsElapsed >= 7) {
    timeOut = true;
    Timer1.stop();
  }
}
