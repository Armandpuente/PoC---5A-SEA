#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Pins des boutons

const int bouttonAllumage = 4;   // Pin du bouton
int etatBouton;            
int dernierEtatBouton = LOW;   
unsigned long boutonPressStart = 0;
bool buttonPressed = false;
const int ledPin = 2; // Pin de la LED
#define DEEP_SLEEP_DELAY 5000 // 5 secondes pour entrer en deep sleep

const int sensorPin = 15;  // pin that the sensor is attached to 
// pin that the LED is attached to 
// variables: 
int sensorValueActuel = 0;   // the sensor value
int sensorValuePrec = 0;   // the sensor value 
int sensorMin = 200;  // minimum sensor value 
int sensorMax = 4095;     // maximum sensor value 

const int buttonNextPin = 12;  // Bouton pour avancer dans la liste
const int buttonPrevPin = 14;  // Bouton pour reculer dans la liste

// Variables pour gérer les boutons
int buttonNextState = 0;
int buttonPrevState = 0;
int lastButtonNextState = 0;
int lastButtonPrevState = 0;

// Déclaration de la liste des musiques sous forme de chaînes de caractères
String musicList[] = {
  "Musique 1",
  "Musique 2",
  "Musique 3",
  "Musique 4",
  "Musique 5"
};
int listSize = sizeof(musicList) / sizeof(musicList[0]); // Taille de la liste

// Index de la liste
int currentIndex = 0;

void setup() {
  // Initialisation du port série
  Serial.begin(9600);
  // initialize LCD 
  lcd.init(); 
  // turn on LCD backlight
  lcd.backlight(); 
  lcd.setCursor(2,0);
  lcd.print("Demarrage en");
  lcd.setCursor(4,1);
  lcd.print("cours !!");
  delay(2000);
  // Initialisation des broches des boutons
  pinMode(buttonNextPin, INPUT);  // Activer la résistance pull-up interne
  pinMode(buttonPrevPin, INPUT);  // Activer la résistance pull-up interne

  pinMode(bouttonAllumage, INPUT);
  pinMode(ledPin, OUTPUT);

if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Réveil par le bouton");
    digitalWrite(ledPin, HIGH);  // L'ESP32 est réveillé, allumer la LED
  } else {
    Serial.println("Démarrage normal");
    digitalWrite(ledPin, HIGH);  // L'ESP32 est réveillé, allumer la LED
  }
}

void eteindre(){
    digitalWrite(ledPin, LOW);  // Éteindre la LED avant d'entrer en deep sleep
    lcd.noBacklight(); //eteindre le retro-eclairage
    lcd.noDisplay();   //eteind le cristaux liquide
    Serial.println("Entrée en deep sleep");
    delay(2000);
    // Configurer le bouton pour réveiller l'ESP32
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);  // Réveiller lorsque le bouton est pressé 
    // Entrer en deep sleep
    esp_deep_sleep_start();
}

int scale(int value){
  // sensorValueActuel = analogRead(sensorPin); 

  // in case the sensor value is outside the range seen during calibration 
  value = constrain(value, sensorMin, sensorMax); 
 
  // apply the calibration to the sensor reading 
  value = map(value, sensorMin, sensorMax, 0, 16); 
 
  value = constrain(value, 0, 16); 

  return value; // Retourner la valeur
}

void affichage(int son, int musique){
  lcd.clear();
  lcd.setCursor(0,1);
  for(int i=0; i < son; i++ ){
      lcd.print("X");
    }
  lcd.setCursor(0,0);
  lcd.print(musicList[musique]);
}

void loop() {
  etatBouton = digitalRead(bouttonAllumage);
  affichage(sensorValueActuel, currentIndex);
  sensorValueActuel = analogRead(sensorPin);
  sensorValueActuel = scale(sensorValueActuel);

  if(sensorValuePrec != sensorValueActuel){
    sensorValuePrec = sensorValueActuel;
    affichage(sensorValueActuel, currentIndex);
  }
  buttonNextState = digitalRead(buttonNextPin);
  buttonPrevState = digitalRead(buttonPrevPin);
  if (buttonNextState == LOW && lastButtonNextState == HIGH) {
    currentIndex++;
    if (currentIndex >= listSize) {
      currentIndex = 0;  // Revenir au début de la liste si on dépasse la fin
    }
    affichage(sensorValueActuel, currentIndex);
  }
  if (buttonPrevState == LOW && lastButtonPrevState == HIGH) {
    currentIndex--;
    if (currentIndex < 0) {
      currentIndex = listSize - 1;  // Revenir à la fin de la liste si on dépasse le début
    }
    affichage(sensorValueActuel, currentIndex);
  }
  lastButtonNextState = buttonNextState;
  lastButtonPrevState = buttonPrevState;

  if (etatBouton != dernierEtatBouton) {
    if (etatBouton == HIGH) {  // Si le bouton est pressé
        boutonPressStart = millis();  // Enregistrer le moment de l'appui
        buttonPressed = true;
    } else {
      buttonPressed = false;  // Réinitialiser lorsque le bouton est relâché
    }
  }
  dernierEtatBouton = etatBouton;

  if (buttonPressed && (millis() - boutonPressStart >= DEEP_SLEEP_DELAY)) {
    eteindre();
  }
  // Petite pause pour éviter le "rebond" des boutons
  delay(200);
}
