import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'grid_display.dart';
import 'model/depths_from_sensor.dart';
import 'package:provider/provider.dart';

class DepthMapGrid extends StatelessWidget{
  static const int _TRAME_SIZE = 20;
  static const int _TRAME_ID = 0x11;

  final BluetoothCharacteristic _characteristic;
  DepthsFromSensor _depthsFromSensor = new DepthsFromSensor();

  DepthMapGrid(this._characteristic){
    if(this._characteristic != null){
      this.setListener();
    }
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

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider<DepthsFromSensor>.value(
      notifier: this._depthsFromSensor,
      child: new GridDisplay()
    );
  }
}