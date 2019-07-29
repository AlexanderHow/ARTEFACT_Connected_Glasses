# ARTEFACT_Connected_Glasses
Projet de lunettes connectées pour mal voyants. Comprend une app mobile sous Flutter qui affichera la carte de profondeur et des programmes à flasher sur le système embarqué pour capter à l'aide d'un capteur ToF la carte de profondeur (à envoyer par BLE)


Programmes à flasher sur une carte Genuino101.<br />
Ils servent pour tester et valider certaines fonctionnalités.<br />
Sensor_SimpleBLE : sert à tester la communication avec le capteur Teraranger evo 64px et à révélé le problème qu'il faut ouvrir un port série à 3Mbps pour communiquer avec le capteur, autrement les données sont corrompues.<br />
EllcieBLE : sert à tester et valider que l'application mobile prend en charge le format et les spécifications du BLE de l'entreprise Ellcie Healthy.<br />

Le capteur peut être testé aussi avec son socle USB avec l'application GUI fournit par Terabee (il fonctionne correctement)

Nous préfèrerons donc la solution utilisant un micro-controleur sous processeur STM32 couplé à la Genuino101

Voir documentation des cartes à processeur Curie Intel et installez la bibliothèque correspondante (dont lib_SSIManager présente dans \ARTEFACT_Connected_Glasses\STM32_to_Genuino101_TREVO64px\v2.0\STM32_ConnectedGlasses)
