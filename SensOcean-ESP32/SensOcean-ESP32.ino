/*********************************************************** 
  
          ASTROLABE EXPEDITIONS - PROGRAMME SENSOCEAN
  Sonde de mesures de temperature et de salitnité de surface
                www.astrolabe-expeditions.org

Composants : 
-----------
base : Carte ESP32 programmé avec arduino ide
Ecran tft 2,8 pouce (carte SD integré)              - Connection SPI
GPS  : neo6                                         - Connection Serie
Conductivité : atlas scientifique EC + isolator     - Connection I2C
Tempéraure : atlas scientifique RTD                 - Connection I2C 
Fuel Gauge pour mesure baterie                      - Connection I2C

Etat du programme : 
-------------------
Programme entierement fonctionnel, 
Note de version : 
V3 et 4 : possibilité de gerer l'alim de la carte EC, RTD, et le retroeclairage de l'ecran.
V4= ajout de fonction deepsleep ESP32 et amélioration du code en particulier sur mise en place de fonction et la possibilité d'allumer les composants par gestion des pin.

Calcul salinité affiché basé sur une temperaure de 15° et les valeurs de EC mesurés (sur la SD est loggué la bonne valeur de EC, ce calcul s'applique uniquement pour l'affichage)

Pour la prochain version : 
------------------------
- ajouter un calcul de salinité propre pour afficher la salinité en psu
- pb avec l'isolator : voir si l'isolator est vraiment utile car actuelement l'isolator n'éteint pas la carte EC et donc consomme de l'énergie 

***********************************************************/

// Version et numero de serie
char numserie[] = "AESO18003"; // Numero de serie de la sonde
char versoft[] = "4.0b";  // version du code

// librairies
#include "SPI.h"                // pour connection ecran bus SPI
#include "Adafruit_GFX.h"       // lib aditionnel pour l'ecran 
#include "Adafruit_ILI9341.h"   // lib de base pour l'ecran tft 2,8"
#include "MAX17043.h"           // lib pour Fuel Gauge Max 17043
#include <TinyGPS++.h>          // GPS
#include <SD.h>                 // pour carte SD

// --- For the tft screen ---  Branchement : vcc sur 3,3v , led sur pinled , mosi : gpio23, Miso : gpio19, sck : gpio18
#define TFT_CS 0
#define TFT_DC 2
#define TFT_RST 27
int pinled = 26;        // branchement de la Led ecran, uniquement si led du tft brancher sur un pin specifique pour pouvoir eteindre et allumer l'ecran manuellement
//#define TFT_MOSI 23 //#define TFT_CLK 18 //#define TFT_MISO 19
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); // Change pins as desired , Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// ---  SD ---  pour la carte SD, mettre une valeur differente de TFT_CS, valeur par default = gpio5 pour la pin CS reconnu automatiquement par la carte ESP32 (miso, sck et mosi = identique a ecran) 
//const int chipSelect = 4; // 4 ou autre, si besoin d'utiliser le gpio5 pour autre choses (ne pas utiliser de base)

//---  GPS  ---  branchement GPS : vcc :3,3v, gnd=gnd, rx gps = TX esp32(gpio17), Tx gps = Rx ESP32(gpio16)              
TinyGPSPlus gps;                           
HardwareSerial Serial2(1);    


// declaration variable globale pour la conductivite et temperature 
int temppin = 14;                   // branchement de la ligne VCC de la carte de temperature.
int ecpin = 15;                     // branchement de la ligne de controle de la carte d'isolation de la carte de conductivité
#define ecAddress 100 
#define rtdAddress 102    
int rtdEcMargin = 20;               // Minimum time period (ms) between end of reading and the next reading
bool rtdReading = false;            // Current reading operation
byte rtdCode=0;                     // Used to hold the I2C response code. 
byte rtdInChar=0;                   // Used as a 1 byte buffer to store in bound bytes from the RTD Circuit.   
char rtdData[20];                   // We make a 20 byte character array to hold incoming data from the RTD circuit. 
int rtdDelay = 600;                 // Used to change the delay needed depending on the command sent to the EZO Class RTD Circuit. 
                                    //  It is the correct amount of time for the circuit to complete its instruction.
bool ecReading = false;             // Current reading operation
byte ecCode = 0;                    // Used to hold the I2C response code.
byte ecInChar = 0;                  // Used as a 1 byte buffer to store in bound bytes from the EC Circuit.
char ecData[48];                    // We make a 48 byte character array to hold incoming data from the EC circuit.
int ecDelay = 1400;                 // Used to change the delay needed depending on the command sent to the EZO Class EC Circuit.
                                    //  It is the correct amount of time for the circuit to complete its instruction.
char *ec;                           // Char pointer used in string parsing.
char *tds;                          // Char pointer used in string parsing.
char *sal;                          // Char pointer used in string parsing.
char *sg;                           // Char pointer used in string parsing.
float f_salin;
//float ec_float;                     //float var used to hold the float value of the conductivity.
//float tds_float;                    //float var used to hold the float value of the TDS.
//float sal_float;                    //float var used to hold the float value of the salinity.
//float sg_float;                     //float var used to hold the float value of the specific gravity.

// definition pour la fonction deepsleep de l'ESP32
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  180       /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;  // utile pour enregistrer un compteur dans la memoire rtc de l'ULP pour un compteur permettant un suivi entre chaque veille





// ****************************************************************************************************************
// ***************** Execution du setup et du code ****************************************************************
// ****************************************************************************************************************


void setup() {
  // INITIALISATION PROTOCOLES ET OBJETS -----------------------------------
  Serial.begin(115200);
  FuelGauge.begin();                         // Fuel gauge
  Wire.begin();                              // begin wire for temp & conduc
  tft.begin();                               // Start ecran
  Serial2.begin(9600, SERIAL_8N1, 16, 17);   // GPS, RX gps connecté sur pin 17(tx2), TX gps connecté sur pin 16(rx2)
  // Test carte SD
  Serial.print("Initializing SD card...");   
    if (!SD.begin(5)) {                      // // see if the card is present and can be initialized, ajouter ici chipSelect ou 5 pour la pin 5 par default
    Serial.println("Card failed, or not present");
    pinMode(pinled, OUTPUT);
    digitalWrite(pinled, HIGH); 
    delay(300);
    tft.setRotation(3); tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE); tft.setTextSize(3);
    tft.setCursor(20,90); tft.print("SD Card failed");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized."); //tft.print("card initialized");
  
  Serial.println("power on");

  // DEFINITION DES PINS POUR LES COMPOSANTS -------------------------------
  pinMode(pinled, OUTPUT);        // definition de la pin ecran
  pinMode(14, OUTPUT);            // pin temperature
  pinMode(15, OUTPUT);            // pin EC

  // ALLUMAGE DES COMPOSANTS -------------------------------------------------
  digitalWrite(14, HIGH);   // temp
  digitalWrite(15, HIGH);   // ec

  // attendre que les carte soit prêtes
  delay(800);               // pas plus court sinon bug de lecture de carte

  // EXECTUTION DU CODE ------------------------------------------------------
  if(bootCount == 0)                         //Run this only the first time
  {
      digitalWrite(pinled, HIGH);    // on allume l'ecran
      ecran_init();                  // on affiche le texte d'intro
      bootCount = bootCount+1;               
      delay(10000);                  // delai pour avoir le temps de lire l'ecran d'accueil
  }else                                      //run this at each loop
  {
      // execution du code de lecture des capteurs et log des données
      temp_mesure(); 
      ec_mesure();
      lecture_GPS();
      calc_sal();
      log_SD();
      digitalWrite(pinled, HIGH);    // on allume le retroeclairage de l'ecran
      affiche_data();
      delay(15000);                  // delai pour avoir le temps de lire l'ecran
  }
  
  //EXTINCTION DES COMPOSANTS ET DE L'ESP32 ----------------------------------
  digitalWrite(14, LOW);  
  digitalWrite(15, LOW);
  digitalWrite(pinled, LOW);
  delay(500);
    
  // endormissement esp32
  Serial.println("power off");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}


void loop() {              // pas de loop car on gere la boucle dans le setup avec la fonction deepsleep de l'esp32
}





// *****************************************************************************************************
// ***************** Definition des fonctions **********************************************************
// *****************************************************************************************************


void temp_mesure()
{
  Wire.beginTransmission(rtdAddress);    // Call the circuit by its ID number.  
  Wire.write('r');                       // Transmit the command that was sent through the serial port.
  Wire.endTransmission();                // End the I2C data transmission. 
  delay(1400);
  Wire.requestFrom(rtdAddress, 20, 1);   // Call the circuit and request 20 bytes (this may be more than we need)
  rtdCode = Wire.read();                 // The first byte is the response code, we read this separately.  
    switch (rtdCode){                    // Switch case based on what the response code is.  
      case 1:                            // Decimal 1.  
        Serial.println("RTD Success");   // Means the command was successful.
        break;                           // Exits the switch case.
      case 2:                            // Decimal 2. 
        Serial.println("RTD Failed");    // Means the command has failed.
        break;                           // Exits the switch case.
      case 254:                          // Decimal 254.
        Serial.println("RTD Pending");   // Means the command has not yet been finished calculating.
        break;                           // Exits the switch case.
      case 255:                          // Decimal 255.
        Serial.println("RTD No Data");   // Means there is no further data to send.
        rtdData[0] = '*';
        rtdData[1] = '*';
        rtdData[2] = 0;
        break;                           // Exits the switch case.
    }
    byte i = 0;                          // Counter used for RTD_data array. 
    while(Wire.available()) {            // Are there bytes to receive.  
      rtdInChar = Wire.read();           // Receive a byte.
      rtdData[i]= rtdInChar;             // Load this byte into our array.
      i+=1;                              // Incur the counter for the array element. 
      if(rtdInChar == 0){                // If we see that we have been sent a null command. 
        i = 0;                           // Reset the counter i to 0.
        Wire.endTransmission();          // End the I2C data transmission.
        break;                           // Exit the while loop.
      }
    }
}


void ec_mesure()
{
    Wire.beginTransmission(ecAddress);   // Call the circuit by its ID number.
    Wire.write('r');                     // r for reading sensor
    Wire.endTransmission();              // End the I2C data transmission. 
    delay(1400);                         // Reading time needing
    Wire.requestFrom(ecAddress, 48, 1);  // Call the circuit and request 48 bytes (this is more than we need)
    ecCode = Wire.read();                // The first byte is the response code, we read this separately.
    byte i = 0;                          // Counter used for EC_data array. 
    while (Wire.available()) {           // Are there bytes to receive.
      ecInChar = Wire.read();            // Receive a byte.
      ecData[i] = ecInChar;              // Load this byte into our array.
      i += 1;                            // Incur the counter for the array element.
      if (ecInChar == 0) {               // If we see that we have been sent a null command.
        i = 0;                           // Reset the counter i to 0.
        Wire.endTransmission();          // End the I2C data transmission.
        break;                           // Exit the while loop.
      }
    }
    switch (ecCode) {                    // Switch case based on what the response code is.
      case 1:                            // Decimal 1.
        Serial.println("EC Success");    // Means the command was successful.
        break;                           // Exits the switch case.
      case 2:                            // Decimal 2.
        Serial.println("EC Failed");     // Means the command has failed.
        break;                           // Exits the switch case.
      case 254:                          // Decimal 254.
        Serial.println("EC Pending");    // Means the command has not yet been finished calculating.
        break;                           // Exits the switch case.
      case 255:                          // Decimal 255.
        Serial.println("EC No Data");    // Means there is no further data to send.
        break;                           // Exits the switch case.
    }
    ec = strtok(ecData, ","); 
    tds = strtok(NULL, ","); 
    sal = strtok(NULL, ","); 
    sg = strtok(NULL, ",");    // Let's pars the string at each comma.
    // ec_float=atof(ec); tds_float=atof(tds);sal_float=atof(sal);sg_float=atof(sg);   //uncomment this section if you want to take the values and convert them into floating point number.

//  Serial.print("EC:");                //we now print each value we parsed separately.
//  Serial.println(ec);                 //this is the EC value.
//  Serial.print("TDS:");               //we now print each value we parsed separately.
//  Serial.println(tds);                //this is the TDS value.
//  Serial.print("SAL:");               //we now print each value we parsed separately.
//  Serial.println(sal);                //this is the salinity value.
//  Serial.print("SG:");                //we now print each value we parsed separately.
//  Serial.println(sg);                 //this is the specific gravity.

}





static void smartDelay(unsigned long ms)               // pour le gps
{
  unsigned long start = millis();
  do
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } while (millis() - start < ms);
}



void lecture_GPS(){
  smartDelay(1000);                                     
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}



void ecran_init()
{
  int duree = TIME_TO_SLEEP/60;
  tft.setRotation(3); tft.fillScreen(ILI9341_BLUE);
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(4);
  tft.setCursor(50,30); tft.println("Astrolabe");
  tft.setCursor(30,80); tft.println("Expeditions");
  tft.setCursor(75,160); tft.setTextColor(ILI9341_WHITE); tft.setTextSize(3); tft.println("SensOcean");
  tft.setCursor(220,220); tft.setTextColor(ILI9341_WHITE); tft.setTextSize(2); tft.print("V - "); tft.print(versoft);
  tft.setCursor(5,225); tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(1); tft.println("www.astrolabe-expeditions.org"); 
  delay(3000);
  tft.fillScreen(ILI9341_WHITE); tft.setTextColor(ILI9341_BLACK); tft.setTextSize(2);
  tft.setCursor(70,15);tft.print("- SENSOCEAN -");
  tft.fillRect(0,40,319,2,ILI9341_YELLOW);
  tft.setCursor(0,70);tft.print("Les donnees seront enregistrees toutes les "); tft.print(duree); tft.print(" min.");
  tft.setCursor(0,140);tft.println("Entre chaque mesure, l'ecran s'eteindra et la sonde ce mettra en veille.");
}


void affiche_data()
{
  // affichage grille fixe et texte de base
  tft.setRotation(3); tft.fillScreen(ILI9341_WHITE); 
  tft.fillRect(0,0,319,50,ILI9341_BLUE);                                                                                         // fond bandeau date et coordonnées
  tft.fillRect(0,50,319,2,ILI9341_YELLOW); tft.fillRect(159,105,2,239,ILI9341_YELLOW); tft.fillRect(0,105,319,2,ILI9341_YELLOW); // quadrillage
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setCursor (12,70); tft.setTextSize(3); tft.print("OCEAN DATA");
  tft.setTextSize(2);
  tft.setCursor(208,75); tft.print("B:"); tft.setCursor(305,75); tft.print("%"); 
  tft.setCursor(195,125); tft.print("Salinity"); 
  tft.setCursor(225,210); tft.print("PSU");
  tft.setCursor(12,125); tft.print("Temperature"); 
  tft.setCursor(35,210); tft.print("Degree C"); 

   // affichage bandeau date et coordonnées
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);  tft.setTextSize(2);
  tft.setCursor (10,10); tft.print(gps.date.year()); 
  tft.setCursor(60,10); tft.print("/"); 
  tft.setCursor(75,10); tft.print(gps.date.month());
  tft.setCursor(100,10); tft.print("/"); 
  tft.setCursor(115,10); tft.print(gps.date.day()); 
  
  tft.setCursor (10,30); tft.print("UT ");
  tft.setCursor(42,30); tft.print(gps.time.hour()); 
  tft.setCursor(65,30); tft.print(":"); 
  tft.setCursor(75,30); tft.print(gps.time.minute()); 
  tft.setCursor(97,30); tft.print(":"); 
  tft.setCursor(109,30); tft.println(gps.time.second());

  tft.setCursor (155,10); tft.print("Lat "); tft.print(gps.location.lat(),6);
  tft.setCursor (155,30); tft.print("Lng "); tft.print(gps.location.lng(),6);

  //Batterie
  tft.setTextColor(ILI9341_BLACK,ILI9341_WHITE); tft.setTextSize(2);
  tft.setCursor(230,75); tft.print(FuelGauge.percent());  
  
  // affichage Data ocean
  tft.setTextColor(ILI9341_BLACK,ILI9341_WHITE);
  tft.setCursor(190,170); tft.setTextSize(3); tft.print(String(f_salin,2));
  tft.setCursor(30,170); tft.setTextSize(3); tft.print(rtdData);
 
}


void log_SD(){                                // Enregistrement sur carte SD
   
  String dataString = ""; // make a string for assembling the data to log:
    dataString += "DATE=";  
    //dataString += String(gps.date.year());
    dataString += "/"; 
    //dataString += String(gps.date.month());
    dataString += "/"; 
    //dataString += String(gps.date.day());
    dataString += "; TIME=";
    //dataString += String(gps.time.hour());
    dataString += ":";
    //dataString += String(gps.time.minute());
    dataString += ":";
    //dataString += String(gps.time.second()); 
    dataString += "; Lat= ";  
    //dataString += String(gps.location.lat(),8);
    dataString += "; Lng = ";  
    //dataString += String(gps.location.lng(),8);
    dataString += "; EC = ";  
    dataString += ec;
    dataString += "; Temp = ";  
    dataString += rtdData;
    
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  //File file = fs.open(path, FILE_WRITE);
  File dataFile = SD.open("/datalog.txt", FILE_APPEND);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");    // tft.setTextColor(ILI9341_BLACK,ILI9341_WHITE); tft.setCursor(190,170); tft.setTextSize(3); tft.print("file Error");
  }
}


void calc_sal()
{
  float Cond =atof(ec)/1000;
  float a1=0.0018, a2=0.755, a3=0.7789;            // valeur approximative pour une eau a 15°C
  f_salin = a1 * (Cond * Cond) + a2 * Cond - a3;   // courbes de salinté=f(cond), mesure grace au fichier ifremer de calcul de salinité
}


