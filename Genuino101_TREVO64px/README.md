# ARTEFACT_Connected_Glasses
Projet de lunettes connectées pour mal voyants. Comprend une app mobile sous Flutter qui affichera la carte de profondeur et des programmes à flasher sur le système embarqué pour capter à l'aide d'un capteur ToF la carte de profondeur (à envoyer par BLE)


Programme de test pour le capteur ToF Teraranger EVO 64px
Permet de tester le bon fonctionnement du capteur et d'évaluer le format de trame envoyé par le capteur en utilisant un socle I2C/UART et l'envoi par BLE des informations de profondeur perçues par le capteur
Cependant il est impossible d'ouvrir un port de communication série à 3 000 000 de bauds par seconde sur Genuino 101, les trames récupérées du capteur sont alors corrompues
Une fonction d'affichage de DEBUG à été laissée pour vous en rendre compte, il suffit de remplacer la fonction d'envoi par BLE par la fonction d'affichage et de commenter tout ce qui concerne le BLE

Le capteur peut être testé aussi avec son socle USB avec l'application GUI fournit par Terabee (il fonctionne correctement)

Nous préfèrerons donc la solution utilisant un micro-controleur sous processeur STM32 couplé à la Genuino101

Voir documentation des cartes à processeur Curie Intel et installez la bibliothèque correspondante