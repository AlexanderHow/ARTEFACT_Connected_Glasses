# mobile_app_connected_glasses

Le code de l'application se trouve en .\mobile_app_connected_glasses\lib<br />
Cette application est développée en Dart sous Flutter et utilise des fonctionnalités bluetooth<br />

## Utilisation :<br />
Lancez l'application<br />
Activez le bluetooth et la localisation si cela n'est pas déjà fait<br />
Appuyez sur la loupe en bas à droite pour lancer la recherche des appareils bluetooth à proximité<br />
Sélectionnez celui réprésentant vos lunettes bluetooth en cliquand sur connect<br />
Attendez que la connexion s'établisse (sur l'écran d'attente)<br />
Observez la carte des profondeur<br />
Cliquez sur les pixel rouge ou orange pour déclencher les vibrations<br />

## Modèle :<br />
**Classe DepthFromSensor :<br />**
Représente la carte des profoneur et présente une interface permettant d'y avoir accès et de la mettre à jour<br />
A modifier si des adaptation sont à apporter à la représentation de la matrice de profondeur.<br />

**Widgets de l'interface :<br />**
* Main :<br /> interface parent à l'application
* BluetoothOff :<br /> partie de l'écran d'acceuil affichée en cas de bluetooth et localisation désactivées sur le téléphone
* GlassesDetection :<br /> partie de l'écran d'accueil qui permet de lancer la recherche bluetooth des appareils bluetooth à proximité
* DepthMap :<br /> écran parent à la carte des profondeur. Permet de se connecter à l'appareil bluetooth
* DepthMapGrid :<br /> partie de l'écran de la carte des profondeur gérant l'affichage dynamique de la carte des profondeur et l'abonnenement à la caractéristique BLE correspondante à la carte des profondeur + l'envoi de la commande BLE pour démarrer le flux.<br /> A modifier si des adaptation sont à apporter à la logique d'abonnenement à la caractéristique BLE ou à la logique de traitement de trames BLE.
* GridDisplay :<br /> partie de l'écran de la carte des profondeur représentant la carte des profondeur

## Références
* Flutter :<br />
[Lab: Write your first Flutter app](https://flutter.dev/docs/get-started/codelab)<br />
[Cookbook: Useful Flutter samples](https://flutter.dev/docs/cookbook)<br />
For help getting started with Flutter, view our <br />
[online documentation](https://flutter.dev/docs), which offers tutorials, <br />
samples, guidance on mobile development, and a full API reference.<br />

* Dart :
https://dart.dev/guides

* Bluetooth :
Pub : https://pub.dev/packages/flutter_blue <br />
Exemple : https://github.com/pauldemarco/flutter_blue/tree/master/example/lib <br />
Exemple : https://medium.com/flutter-community/flutter-adding-bluetooth-functionality-1b9715ccc698
