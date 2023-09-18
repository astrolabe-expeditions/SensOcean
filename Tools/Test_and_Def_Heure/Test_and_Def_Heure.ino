/*
Programme pour la mise à l'heure d'une horloge SD3031 ou DS3231: 
1- ouvrir dans un navigateur un serveur de temps, par exemple sur le site https://time.is
2 - charger le programme et ouvrir le moniteur serie 
3 - Entrer la date au format YYYY-MM-DD HH:MM:SS
4 - Appuyer sur la touche entrée pour synchroniser (prévoir d'appuyer sur entrée 1s avant l'heure exacte (durée d'exécution du code)

Format date : 
Exemple : Taper 2023-06-20 15:23:45 pour mettre à la date du 20 juin 2023 à 15h23min45s"

*/



// Pour horloge RTC
#include <DS3231.h>

DS3231 Clock;
bool Century = false;
bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;

byte year, month, date, DoW, hour, minute, second;
int state = 1; // 0 pour set time, 1 pour get time

int donnee; // données reçues sur la liaison série

const int led = 25;
const int en5v = 4;


void setup() {
  Serial.begin(115200);

  pinMode(led, OUTPUT);
  pinMode(en5v, OUTPUT);
  digitalWrite(led, HIGH);
  digitalWrite(en5v, HIGH);
  delay(400);


  //Initilisation wire pour rtc
  Wire.begin();

}

void loop() {
  Serial.println("presser entrer pour lancer la synchro");
  if (Serial.available() > 0) {
    // on lit la données reçue (dans la variable "donnee")
    donnee = Serial.read();
    // set time
    Clock.setSecond(00);//Set the second
    Clock.setMinute(53);//Set the minute
    Clock.setHour(17);  //Set the hour
    Clock.setDoW(5);    //Set the day of the week
    Clock.setDate(26);  //Set the date of the month
    Clock.setMonth(5);  //Set the month of the year
    Clock.setYear(23);  //Set the year (Last two digits of the year)
    Clock.setClockMode(false); // pour avoir le format 24h
    Serial.println("synchro ok");
  }
  else {
    int second, minute, hour, date, month, year;

    // lecture de l'horloge rtc
    second = Clock.getSecond();
    minute = Clock.getMinute();
    hour = Clock.getHour(h12, PM);
    date = Clock.getDate();
    month = Clock.getMonth(Century);
    year = Clock.getYear();
    Serial.println("");
    Serial.print(year); Serial.print("/"); Serial.print(month); Serial.print("/"); Serial.print(date); Serial.print(" ; ");
    Serial.print(hour); Serial.print(" : "); Serial.print(minute); Serial.print(" : "); Serial.println(second);
    delay(1000);
  }

}
