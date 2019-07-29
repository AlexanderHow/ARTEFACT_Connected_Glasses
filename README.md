# ARTEFACT_Connected_Glasses
Projet de lunettes connectées pour mal voyants. Comprend une application mobile sous Flutter qui affichera la carte de profondeur et des programmes à flasher sur le module embarqué pour capter à l'aide d'un capteur ToF la carte de profondeur (à envoyer par BLE par la main board)
<br />

## Architecture Projet :<br />

* **/Genuino101_TREVO64px** :<br /> 
Contient des programmes à flasher sur une cartes Genuino 101. Ces programmes sont des simulations ou use cases servant à la validation de certaines parties du projet.

  * **/useCase_Sensor_SimpleBLE** :<br />
Contient le code à flasher sur une carte Genuino101. Il met en oeuvre la récupération de trames venant du capteur ToF Teraranger Evo 64px et l'envoi de ses informations par BLE selon la norme GATT.<br /> Un soucis de performances empèche cette carte d'ouvrir un port série à 3Mbps (fréquence nécessaire pour lire les trames du capteur correctement). Par conséquent les trames lues sont corrompues sur cette simulation. Une fonction d'affichage sur le port série permet de se rendre compte de ce problème de fréquence dont il faut tenir compte à l'avenir.

  * **/useCase_EllcieBLE** :<br />
Contient le code à flasher sur une carte Genuino101. Il met en oeuvre l'utilisation du BLE selon les spécifications utilisées par l'entreprise Ellcie Healthy (un service de contrôle avec une caractéristique pour démarrer le streaming de données + un service et sa caractéristique pour diffuser le flux de données).<br /> Il sert à valider le bon fonctionnement de l'application mobile en accord ces spécifications BLE et à illustrer un template de code diffusant selon ces spécifications une trame contenant des informations de profondeur d'un capteur ToF (générées aléatoirement puis formattées pour ce test/use case).<br /> Les communications BLE avec l'application mobile en accord avec les spécifications définies ont été validés.

* **/mobile_app** :<br />
Contient le projet Flutter pour l'application mobile cross-plateforme IoS et Android. Ses fonctionnalités sont :<br /> 
  - Recherche des appareils bluetooth à proximité
  - Connexion à un appareil bluetooth
  - Abonnement automatique à la caratéristique définit pour communiquer les données du capteur ToF + envoi automatique de la commande pour démarrer le flux BLE (en respectant les spécifications BLE de l'entreprise Ellcie Healthy)
  - Affichage et mise à jour d'une carte de profondeurs en fonction des données du capteur ToF
  - Evenement tactile de vibration au toucher un pixel perçu trop proche du capteur

* **/STM32_to_Genuino101_TREVO64px** :<br /> 
Contient les programmes à flasher sur une carte Nucleo-64 L476FG et sur une carte Genuino101. La partie sur la carte Nucleo-64 représente le module à attacher aux lunettes et gérant la réception des données du cpateur ToF, la partie sur la carte Genuino101 représente une simulation de la main board des lunette (avec le BLE).<br /> Cette partie est séparer en deux versions. La 2ème contient l'implémentation finale et correcte du module sur la carte Nucleo-64

  * **/v1.0** : <br />
Contient la première version représentant le use case suivant : Le module sur la carte Nucleo-64 récupère les données du capteur Teraranger Evo 64px et les parse pour les envoyer par port série à la carte Genuino101. Le module sur la carte Genuino101 récupère les informations données par la carte Nucleo-64 et les diffuse par BLE pour être visualisées sur l'application mobile. Les échanges entre les deux cartes se font par ports séries avec des marqueurs de début de trame pour reconnaître le format/usage de ces dernières.<br /> Cette version ne prend pas en compte le protocole SSI, elle met simplement en oeuvre une communication complète de puis le module de la Nucleo-64 à la Genuino101 jusqu'à l'application mobile.<br /> (Il faut flasher la carte Genuino101 avant la carte Nucleo-64 pour les synchroniser).
  
  * **/v2.0** : <br />
Contient la deuxième version représentant le use case suivant : Le module sur la carte Genuino101 envoie les commandes SSI Query, GetConfiguration et CreateObserver, le module sur la Nucleo-64 les reconnait et répond à chacune de ces commandes. Une fois la commande CreateObserver traitée, le module de la Nucleo-64 envoie des commandes ManyData à intervalle régulier, après avoir récupéré les 64 pixels de profondeur du capteur ToF. Le module sur la Genuino101 reconnait les commandes ManyData et les parse en une matrice de profondeur.<br /> La communication BLE de fin à été retirée pour ce use case car solicitant trop la carte Genuino101 et à été remplacée par l'affichage de logs sur le port série USB. De cette manière le traitement des communications SSI est davantage explicité et mit en avant pour la validation.<br /> (Il faut flasher la carte Nucleo-64 avant la carte Genuino101 pour les synchronniser, un delay au démarrage de 4 secondes est présent du côté de la Genuino101 pour vous laisser le temps d'ouvrir le port série USB associé à cette dernière pour voir les logs complets).<br /> Les communications SSI et le traitement des données en provenance du capteur Teranranger evo 64px ont été validés dans cette version finale.<br /> Pour une implémentation, sur un processeur STM32, similaire au module sur la carte Nucleo-64, il faudra veiller à utiliser les bon ports séries (1 et 4) ou les redéfinir via la bibliothèque HardwareSerial utilisée au début du programme pour la Nucleo-64.

  * **/v2.0/STM32_ConnectedGlasses/lib_SSIManager** : <br />
Est la bibliothèque créée et utilisée pour gérer les communications SSI et formatter les trames à envoyer. (inclue un fichier de configuration à ne pas réimporter)

<br />

## Softwares et Langages utilisés :
* Développement mobile : Android Studio avec les plugins Flutter et Dart
* Développement embarqué : Arduino IDE avec les bibliothèques adaptées aux boards cibles et en C


*Plus de détails sont fournis dans les différentes sous parties du projet quant aux références et guide d'utilisation/installation
