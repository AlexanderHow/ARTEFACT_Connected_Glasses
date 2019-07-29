# ARTEFACT_Connected_Glasses
Projet de lunettes connectées pour mal voyants. Comprend une app mobile sous Flutter qui affichera la carte de profondeur et des programmes à flasher sur le système embarqué pour capter à l'aide d'un capteur ToF la carte de profondeur (à envoyer par BLE)

Sous partie contenant les programmes à flasher sur les cibles embarqués<br />
Les boards utilisées sont :
- Pour la partie module des lunettes, une board Nucleo 64 L476FG sous processeur STM32
- Pour la partie simulation de la main board des lunettes, une dev. board Genuino 101

En deux versions :
- v1.0 :<br /> Incluant le parsing des données envoyées par le capteur ToF, la communication simple (sans SSI) entre module et main board et la diffusion simple par BLE de ces données.
- v2.0 :<br /> Incluant le parsing des données envoyées par le capteur ToF, la communication SSI entre module et main board. Le BLE est remplacé par du logs sur le port série USB côté Genuino101 pour expliciter les échanges SSI. Le BLE n'est pas primordial car l'objectif était de développer un module (celui pour la carte Nucleo-64, avec un processeur STM-32) pouvant interpréter les données du capteur ToF et communiquer avec une main board par le protocole SSI.

## Infos générale :<br />
Une trame venant du capteur suis le schémas suivant : <br />
{ 1 Byte de header, 64 * 2 Bytes pour les mesures de distances sur 64 pixels, N Bytes de padding valant 0x80, 8 Bytes de CRC32 }<br />
Pour les 64 mesures de distances, elles pèsent 2 Bytes selon le format suivant :<br />
High byte = { 1XXV VVVV } et Low byte = { 1VVV VVVV }, seuls les V constituent le résultat exacte. Donc distance = ((High byte & 0x1F) << 7) | (Low byte & 0x7F)<br />
La valeur de header 0x11 correspond à une trame concernant les distances<br />
Attention ! L'usage du socle I2C/UART fournit avec le capteur nécessite d'ouvrir un port série à 3 000 000 Bauds / sec (ou Baud rate)<br />

Des commandes permettent de configurer le mode de fonctionnement du capteur :
- Distance Print :<br /> The sensor outputs 64 distance values = 0x00 11 02 4C
- Distance AmbientPrint (Default) :<br /> The sensor outputs 64 distance valuesand 64 ambient values = 0x00 11 03 4B
- Close Range Mode(Default) :<br /> The sensor takes 2 subsequent frames at different light modulation frequencies and builds the final image by picking the best pixels of the 2 frames. = 0x00 21 01 BC
- Fast Mode :<br /> If performance is driven by the readings peed, the sensor can be set to workin this mode. = 0x00 21 02 B5
Malheureusment aucune commande ne permet de réajuster le baud rate du socle utilisé

Après récupération des 64 distances (entre 0 et 5000 mm), elles sont ramenées à des valeurs entre 1 et 254 (pour correspondre à une information sur 1 Byte)

## Concernant la v1.0 : <br />
Le module de la Nucleo-64 commence par envoyer la trame suivante : { 255 nombre_de_lignes_de_la_matrice_de_profondeur nombre_de_colonnes_de_la_matrice_de_profondeur }<br />
Et ensuite après chaque trame récupérée et interprétée du capteur sur le port série 1, le module envoie :<br /> { 0 les_64_valeurs_de_distances_comprises_entre_1_et_254 } sur le port série 4<br />

Le programme pour la Genuino101 simule la main board des lunettes et intégre la capacité de faire du BLE.<br />
Ce programme reçoit sur le port série 1, ce que le module précédent envoie sur son port série 4, càd les 64 valeurs de distances
puis envoie la matrice de profondeur ligne par ligne sur une caractéristique BLE selon le format suivant :<br />
{ 17 num_séquence taille_ligne les_valeurs_de_distance_de_la_ligne_courante padding_de_0 } = 20 Bytes<br />

Certains soucis de performances vis à vis de la Genuino 101 sont à noter
- Impossible d'ouvrir un port série avec un Baud rate de 3 000 000 de bauds par seconde, ce qui fait qu'elle soit en retard sur la lecture de ce qu'envoie le module STM32 et qu'elle saute quelques trames
- Ce ralentissement sature les buffers des ports série et BLE
- Cette saturation provoque des déconnexions de la connexion bluetooth

## Concernant la v2.0 : <br />
Les communications entre les deux cartes se fait en suivant le protocole SSI suivant (http://www.janding.fi/iiro/papers/SSI%20protocol%20specification_12.pdf) <br />
Du logs sur le port série USB de la Genuino101 remplace le BLE pour expliciter davantage les échanges SSI, un delay de 4 secondes au démarrage de cette dernière vous permet d'avoir le temps d'ouvrir le moniteur série associé.
Le dossier contenant le programme pour la carte Nucleo-64 (processeur STM32) inclut une blibliothèque custom qui regroupe des fonctionnalités communes au niveau du traitement des trames SSI et de leur formattage.

## Concernant la lib_SSIMananger : <br />
lib_SSIManager is a lib to manage the ssi commands, <br />
you must put it in your arduinoIDE librairies directory and import SSIManager.h through arduinoIDE Sketch \> Import<br />
see a tutorial to import custom librairies :<br />
https://www.arduino.cc/en/Hacking/LibraryTutorial <br />

## Références :<br />
Documentation du capteur : https://www.robotshop.com/media/files/content/t/ter/pdf/teraranger-evo-64px-tof-rangefinder-datasheet2.pdf <br />
Exemple de code pour un capteur single point similaire : https://github.com/Terabee/sample_codes/blob/master/Arduino/TeraRanger_single_point_ArduinoMega_UART.ino <br />
Mappage de pins sur la board Nucleo 64 utilisée : http://hebergement.u-psud.fr/villemejane/IOGS/ProTIS/ProTIS_Nucleo_2018.pdf <br />
Getting started STM32 + Arduino IDE : https://time4ee.com/articles.php?article_id=74 et https://www.instructables.com/id/Quick-Start-to-STM-Nucleo-on-Arduino-IDE/ <br />
Communication série STM32 : https://time4ee.com/articles.php?article_id=85 <br />
