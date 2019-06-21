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

  void _startVibration(PointerEnterEvent details){
    //Vibration.hasVibrator()
    Vibration.vibrate();
  }

  void _stopVibration(PointerExitEvent details){
    Vibration.cancel();
  }

  Color getColorDepth(int depth) {
    if(depth >=0 && depth <= 42){
      return Colors.red;
    }else if(depth >= 43 && depth <= 85){
      return Colors.deepOrangeAccent;
    }else if(depth >= 86 && depth <= 128){
      return Colors.yellowAccent;
    }else if(depth >= 129 && depth <= 171){
      return Colors.green;
    }else if(depth >= 172 && depth <= 214){
      return Colors.lightBlueAccent;
    }else if(depth >= 215 && depth <= 255){
      return Colors.indigo;
    }else{
      return Colors.black54;
    }
  }

  Widget buildGridTile(int depth, int lineSize, int colSize, BuildContext context){
    return Listener(
        onPointerEnter: (depth >= 0 && depth <= 85) ? _startVibration : null ,
        onPointerExit: (depth >= 0 && depth <= 85) ? _stopVibration : null ,
        child: Container(
          height: (MediaQuery.of(context).size.width  * 0.9) / lineSize,
          width: (MediaQuery.of(context).size.width  * 0.9) / colSize,
          color: this.getColorDepth(depth),
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
          return buildGridTile(Provider.of<DepthsFromSensor>(context).getDepth(index),
              Provider.of<DepthsFromSensor>(context).depthMatrix[0].length,
              Provider.of<DepthsFromSensor>(context).depthMatrix.length,
              context);
        }),
      )
    );
  }
}