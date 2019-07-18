# ARTEFACT_Connected_Glasses
Projet de lunettes connectées pour mal voyants. Comprend une application mobile sous Flutter qui affichera la carte de profondeur et des programmes à flasher sur le système embarqué cible pour capter à l'aide d'un capteur ToF la carte de profondeur (à envoyer par BLE)

Architecture Projet : 
> mobile_app/ :
Contient l'application mobile sous Flutter. Cette application est cross-plateforme IoS et Android.
Ses fonctionnalitées sont : 
- Recherche des appareils bluetooth à proximité
- Connexion à un appareil bluetooth
- Abonnement automatique à la caratéristique définit pour communiquer les données du capteur ToF
- Affichage et mise à jour d'une carte de profondeurs en fonction des données du capteur ToF
- Evenement tactile de vibration quand on touche un pixel perçu trop proche du capteur

> STM32_to_Genuino101_TREVO64px/ : 
Contient une programme pour cible avec processeur STM32 (en l'occurence ici une board Nucleo64 L476RG).
Ce programme récupère les trames du capteur ToF Teraranger EVO 64px et les parse puis transmet par un port serie selon un format de trame et de protocole SSI plus simple et déjà formatté
Contient aussi un programme d'essai pour une dev. board Genuino 101 afin de simuler la main board des lunette contenant un module BLE.
Ce programme récupère la trame simplifiée et formattée du module sous processeur STM32 et les envoie par BLE à l'application mobile.

> Genuino101_TREVO64px/ :
Contient le premier programme de test du capteur ToF Teraranger EVO 64px pour une carte Genuino 101.
Ce programme récupère les trames envoyées par le capteur et les transmet par BLE à l'application mobile après les avoir interprété.
Un soucis au niveau des performances de cette carte fait que la récupèration de données depuis le capteur est impossible ou du moins est corrompue.
Une fonction d'affichage marquée DEBUG à été laissée pour permettre de voir la corruption des trames


Softwares et Langages utilisés :
> Développement mobile : Flutter et Dart
> Développement embarqué : Arduino IDE avec les bibliothèques adaptées aux boards cibles et en C


*Plus de détails sont fournis dans les différentes sous parties du projet quant aux références et guide d'utilisation/installation
