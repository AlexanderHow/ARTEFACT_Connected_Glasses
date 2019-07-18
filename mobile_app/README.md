# mobile_app_connected_glasses

Le code de l'application se trouve en .\mobile_app_connected_glasses\lib
Cette application est développée en Dart sous Flutter et utilise des fonctionnalités bluetooth

Utilisation :
Lancez l'application
Activez le bluetooth et la localisation si cela n'est pas déjà fait
Appuyez sur la loupe en bas à droite pour lancer la recherche des appareils bluetooth à proximité
Sélectionnez celui réprésentant vos lunettes bluetooth en cliquand sur connect
Attendez que la connexion s'établisse (sur l'écran d'attente)
Observez la carte des profondeur
Cliquez sur les pixel rouge ou orange pour déclencher les vibrations

Modèle :
> Classe DepthFromSensor :
Représente la carte des profoneur et présente une interface permettant d'y avoir accès et de la mettre à jour

Widgets de l'interface :
> Main : interface parent à l'application
> BluetoothOff : partie de l'écran d'acceuil affichée en cas de bluetooth et localisation désactivées sur le téléphone
> GlassesDetection : partie de l'écran d'accueil qui permet de lancer la recherche bluetooth des appareils bluetooth à proximité
> DepthMap : écran parent à la carte des profondeur. Permet de se connecter à l'appareil bluetooth
> DepthMapGrid : partie de l'écran de la carte des profondeur gérant l'affichage dynamique de la carte des profondeur et l'abonnenement à la caractéristique BLE correspondante à la carte des profondeur
> GridDisplay : partie de l'écran de la carte des profondeur représentant la carte des profondeur

## Références
> Flutter :
- [Lab: Write your first Flutter app](https://flutter.dev/docs/get-started/codelab)
- [Cookbook: Useful Flutter samples](https://flutter.dev/docs/cookbook)
For help getting started with Flutter, view our 
[online documentation](https://flutter.dev/docs), which offers tutorials, 
samples, guidance on mobile development, and a full API reference.

> Dart :
https://dart.dev/guides

> Bluetooth :
Pub : https://pub.dev/packages/flutter_blue
Exemple : https://github.com/pauldemarco/flutter_blue/tree/master/example/lib
Exemple : https://medium.com/flutter-community/flutter-adding-bluetooth-functionality-1b9715ccc698

