# SensOcean

_SensOcean est un programme de sciences participative d’océanographie physique qui permet d'étudier l'océan global, les courants marins et le climat grâce à la particiption de bateau volontaire en haute mer. Ces bateaux vont installer l'instrument de mesure "kit SensOcean" pour enregistrer les paramètres physiue de l'eau durant leurs voyages._

__Le kit SensOcean permet de mesurer :__
- Température et Salinité de l'eau de mer
- température et pression atmosphérique
- position GPS des points de mesures et du trajet, ainsi que la date et l'heure.


__Les liens du programme :__
- Site web du programme : https://www.astrolabe-expeditions.org/programme-de-sciences/sensocean/
- Wikifactory pour construire l'instrument : https://wikifactory.com/+astrolabeexpeditions/sensocean
- Github du code de l'instrument de mesure : https://github.com/astrolabe-expeditions/SensOcean﻿
- Schéma electronique de l'instrument : https://oshwlab.com/Astrolabe-Expeditions/sensocean_instrument


__Ce github contient :__
- Le logiciel pour faire fonctionne l'instrument
- Les librairies pour que le logiciel fonctionne
- Les étapes de configuration et d'installation de l'instrument
- Les codes de calibrations, et de mise à l'heure
- Le manuel d'utilisation de l'instrument (pdf)
- le modele de fichier config.txt


## Configuration et utilisation de l'instrument : 
Une fois l'instrument assemblé selon la configuration décrite ici : https://wikifactory.com/+astrolabeexpeditions/sensocean, il faudra configurer et calibrer l'instrument

### 1- installer Arduino, les librairies, les drivers
Toutes les librairies utiles se trouve dans le dossier SO_libraries de ce github
Les procédurers pour ces étapes sont décrite ici : https://cedriccoursonopensourceoceanography.notion.site/Programmer-SensOcean-b69c2cf5203f4e7b8d01bfecacec98ce

### 2- Mettre à l'heure l'horloge RTC
L'horloge de l'instrument doit être mise à l'heure universel, pour cela on peut utiliser le fichier Def_heure.ino
Procédure : 
- on ouvre un site de server de temps universel, par exemple : https://time.is/fr/GMT
- Dans le fichier Def_heure.ino on rentrer la date et l'heure. Pour les minutes on ajoutera 2min à l'heure réel au moment de modifier le fichier. 
- On charge le fichier dans l'instrument SensOcean
- On ouvre le moniteur serie pour voir s'afficher l'heure toute les secondes
- Dans la moniteur serie, à l'emplacement prévu pour écrire une instruction, on tapera "entrée" à la seconde exacte ou s'afficher sur le server de temps l'heure entrer dans Def_heure.

### 3- Calibrer l'instrument
La sonde Atlas Scientific qui est utiliser dans cet instrument doit être calibrer. 
La procédure pour cela se trouve ici : https://cedriccoursonopensourceoceanography.notion.site/Calibration-conductivit-33b4078230a4473a9252c3aaaad418b1

### 4- Installer le logiciel SensOcean
Une fois l'instrument calibré, et la carte SD configurer, il faut installer le logiciel SensOcean dans l'instrument. 
Il s'agit du fichier : SensOcean-ESP32.ino
L'ouvrir avec l'environnement de programmation Arduino et le charger sur l'instrument (via um cable USB).

### 5- Installation de la carte SD et création du fichier config
Se procurer une carte mini-SD, et copier les valeurs présentées dans le fichier config_MODELE.txt sur la carte SD dans un fichier intitulé config.txt
Modifier les paramètres modifiable et enregistrer ce fichier config.txt sur la carte SD. 
