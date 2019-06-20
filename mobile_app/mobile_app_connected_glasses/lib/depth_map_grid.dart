import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'model/depths_from_sensor.dart';
import 'package:provider/provider.dart';
import 'package:vibration/vibration.dart';

class DepthMapGrid extends StatelessWidget{
  static const int _TRAME_SIZE = 20;
  static const int _TRAME_ID = 0x11;

  final BluetoothCharacteristic _characteristic;
  DepthsFromSensor _depthsFromSensor = new DepthsFromSensor();

  DepthMapGrid(this._characteristic){
    this.setListener();
  }

  void setListener() async {
    await this._characteristic.setNotifyValue(true);
    this._characteristic.value.listen((value) {
      if(value.length == _TRAME_SIZE){
        int idTrame = value[0];
        int numSequence = value[1];
        int sizeRow = value[2];
        if(idTrame == _TRAME_ID){
          List<int> row = new List<int>();
          for(int i = 0; i < sizeRow; ++i){
            row.add(value[3+i]);
          }
          this._depthsFromSensor.updateDepthMatrix(numSequence, row);
        }
      }
    });
  }

  void _startVibration(PointerEnterEvent details/*, int depth*/){
    //if (Vibration.hasVibrator() != null) {
      Vibration.vibrate();
    //}
  }

  void _stopVibration(PointerExitEvent details){
    Vibration.cancel();
  }

  //TODO : event vibrate if depth < X + moduler couleur en fonction de depth
  Widget buildGridTile(int depth){
    return Listener(
        onPointerEnter: _startVibration,
        onPointerExit: _stopVibration,
        child: Container(
          color: Colors.lightBlueAccent,
        )
    );
  }

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider<DepthsFromSensor>.value(
      notifier: this._depthsFromSensor,
      child: new GridView.count(
        crossAxisCount: Provider.of<DepthsFromSensor>(context).depthMatrix.length,
        physics: new NeverScrollableScrollPhysics(),
        primary: true,
        children: List.generate(Provider.of<DepthsFromSensor>(context).totalSize, (index){
          return buildGridTile(Provider.of<DepthsFromSensor>(context).getDepth(index));
        }),
      )
    );
  }
}