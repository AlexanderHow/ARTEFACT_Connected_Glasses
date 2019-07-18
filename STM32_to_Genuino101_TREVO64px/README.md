# ARTEFACT_Connected_Glasses
Projet de lunettes connectées pour mal voyants. Comprend une app mobile sous Flutter qui affichera la carte de profondeur et des programmes à flasher sur le système embarqué pour capter à l'aide d'un capteur ToF la carte de profondeur (à envoyer par BLE)


#Sous partie contenant les programmes à flasher sur les cibles embarqués
Les boards utilisées sont :
- Pour la partie module des lunettes, une board Nucleo 64 L476FG sous processeur STM32
- Pour la partie simulation de la main board des lunettes, une dev. board Genuino 101


#STM32_ConnectedGlasses/ :
Programme pour le module des lunettes. Permet de récolter les données envoyées par le capteur ToF teraranger evo 64px et de les formatter pour les retransmettre sur un port serie.

Une trame venant du capteur suis le schémas suivant :
{ 1 Byte de header, 64 * 2 Bytes pour les mesures de distances sur 64 pixels, N Bytes de padding valant 0x80, 8 Bytes de CRC32 }

Pour les 64 mesures de distances, elles pèsent 2 Bytes selon le format suivant :
High byte = { 1XXV VVVV } et Low byte = { 1VVV VVVV }, seuls les V constituent le résultat exacte. Donc distance = ((High byte & 0x1F) << 7) | (Low byte & 0x7F)

La valeur de header 0x11 correspond à une trame concernant les distances

Attention ! L'usage du socle I2C/UART fournit avec le capteur nécessite d'ouvrir un port série à 3 000 000 Bauds / sec (ou Baud rate)

Des commandes permettent de configurer le mode de fonctionnement du capteur :
Distance Print : The sensor outputs 64 distance values = 0x00 11 02 4C
Distance AmbientPrint ​(Default) : The sensor outputs 64 distance valuesand 64 ambient values = 0x00 11 03 4B
Close Range Mode(Default) : The sensor takes 2 subsequent frames at different light modulation frequencies and builds the final image by picking the best pixels of the 2 frames. = 0x00 21 01 BC
Fast Mode : If performance is driven by the readings peed, the sensor can be set to workin this mode. = 0x00 21 02 B5
Malheureusment aucune commande ne permet de réajuster le baud rate du socle utilisé

Après récupération des 64 distances (entre 0 et 5000 mm), elles sont ramenées à des valeurs entre 1 et 254 (pour correspondre à une information sur 1 Byte)

Le module commence par envoyer la trame suivante : { 255 nombre_de_lignes_de_la_matrice_de_profondeur nombre_de_colonnes_de_la_matrice_de_profondeur }
Et ensuite après chaque trame récupérée et interprétée du capteur sur le port série 1, le module envoie : { 0 les_64_valeurs_de_distances_comprises_entre_1_et_254 } sur le port série 4


#Genuino_ConnectedGlasses/ :
Programme simulant la main board des lunettes et intégrant la capacité de faire du BLE
Ce programme reçoit sur le port série 1, ce que le module précédent envoie sur son port série 4, càd les 64 valeurs de distances
puis envoie la matrice de profondeur ligne par ligne sur une caractéristique BLE selon le format suivant :
{ 17 num_séquence taille_ligne les_valeurs_de_distance_de_la_ligne_courante padding_de_0 } = 20 Bytes

Certains soucis de performances vis à vis de la Genuino 101 sont à noter
- Impossible d'ouvrir un port série avec un Baud rate de 3 000 000 de bauds par seconde, ce qui fait qu'elle soit en retard sur la lecture de ce qu'envoie le module STM32 et qu'elle saute quelques trames
- Ce ralentissement sature les buffers des ports série et BLE
- Cette saturation provoque des déconnexions de la connexion bluetooth



#Références :
Documentation du capteur : https://www.robotshop.com/media/files/content/t/ter/pdf/teraranger-evo-64px-tof-rangefinder-datasheet2.pdf
Exemple de code pour un capteur single point similaire : https://github.com/Terabee/sample_codes/blob/master/Arduino/TeraRanger_single_point_ArduinoMega_UART.ino
Mappage de pins sur la board Nucleo 64 utilisée : http://hebergement.u-psud.fr/villemejane/IOGS/ProTIS/ProTIS_Nucleo_2018.pdf
Getting started STM32 + Arduino IDE : https://time4ee.com/articles.php?article_id=74 et https://www.instructables.com/id/Quick-Start-to-STM-Nucleo-on-Arduino-IDE/
Communication série STM32 : https://time4ee.com/articles.php?article_id=85