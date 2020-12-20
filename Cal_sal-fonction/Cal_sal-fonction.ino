// paramètre t et C à récupérer lors des mesures des sondes

// déclaration initiale des variables de la fonction cal_sal (Aminot A., Kérouel R. (2004), Hydrologie des écosystèmes marins. Paramètres et analyses. Cf pages 74-78)
int Rp = 1;             
float a[] = {0.0080, -0.1692, 25.3851, 14.0941, -7.0261, 2.7081};
float b[] = {0.0005, -0.0056, -0.0066, -0.0375, 0.0636, -0.0144};
float c[] = {0.6766097, 0.0200564, 0.0001104259, -6.9698E-07, 1.0031E-09};
float k = 0.0162;
float rt, R_t, S, modA, modB;


void setup() {
  Serial.begin(115200);

}

void loop() {
 cal_sal(15, 42.914);
 cal_sal_affiche(15, 42.914);
}


void cal_sal(float t, float C){         // fonction sinplifié à intégrer dans un code
  // calcul intermédiaire 
  int i;
  rt=0;                                 // réinitilisation entre chaque boucle
  for (i=0; i<5; i++){                  // calcul de rt
    rt += c[i]*pow(t,float(i));
  }

  R_t= C /(42.914*rt);                  // calcul de R_t

  modA =0; modB=0;                      // réinitilisation entre chaque boucle
  for (i=0; i<6;i++){                   // cal modA et modB
    modA += a[i]*pow(R_t,float(i/2));
    modB += b[i]*pow(R_t,float(i/2));
  }

  //cal salinité
  S = modA+((t-15)/(1+k*(t-15)))*modB;
  Serial.print("Salinité = "); Serial.println(S,4);

}

void cal_sal_affiche(float t, float C){      // fonction avec affichage complet sur le port serie
    Serial.println("--------------------- Fonction CALCUL Salinité -------------------------");

  Serial.println("----- paramètres intitiaux ------ ");
  Serial.print(" A = ");
  int i;
  for (i = 0; i < 6; i = i + 1) {
  Serial.print(a[i],4);Serial.print(" ; ");
  }
  Serial.println("  ");
  
  Serial.print(" B = ");
  for (i = 0; i < 6; i = i + 1) {
  Serial.print(b[i],4);Serial.print(" ; ");
  }
  Serial.println("  ");

  Serial.print(" C = ");
  for (i = 0; i < 5; i = i + 1) {
  Serial.print(c[i],14);Serial.print(" ; ");
  }
  Serial.println("  ");
  Serial.print(" K : "); Serial.println(k,4);
  Serial.print("  ");

  Serial.println("----- Calcul intermédiaire ----- ");
  rt=0;                                 // réinitilisation entre chaque boucle
  for (i=0; i<5; i++){
    rt += c[i]*pow(t,float(i));
  }
  Serial.print("rt = ");Serial.println(rt,10);
  
  R_t= C /(42.914*rt);
  Serial.print("R_t = ");Serial.println(R_t,10);

  // cal modA et modB
  modA =0; modB=0;                      // réinitilisation entre chaque boucle
  for (i=0; i<6;i++){
    modA += a[i]*pow(R_t,float(i/2));
    modB += b[i]*pow(R_t,float(i/2));
  }
  Serial.print("modA = ");Serial.println(modA,15);
  Serial.print("modB = ");Serial.println(modB,15);

  Serial.println("----- Calcul Salinité ----------- ");
  //cal salinité
  S = modA+((t-15)/(1+k*(t-15)))*modB;

  Serial.print("Salinité = "); Serial.println(S,3);


  delay(1000);
  while(1){}
  
}
