import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:vibration/vibration.dart';

import 'model/depths_from_sensor.dart';

class GridDisplay extends StatelessWidget{

  GridDisplay();

  void _startVibration(){
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
    return GestureDetector(
        onTap: (depth >= 0 && depth <= 85) ? _startVibration : null ,
        onLongPress: (depth >= 0 && depth <= 85) ? _startVibration : null ,

        child: Container(
          height: (MediaQuery.of(context).size.width  * 0.9) / lineSize,
          width: (MediaQuery.of(context).size.width  * 0.9) / colSize,
          color: this.getColorDepth(depth),
        )
    );
  }

  @override
  Widget build(BuildContext context) {
    DepthsFromSensor depthsFromSensor = Provider.of<DepthsFromSensor>(context);
    return new GridView.count(
      crossAxisCount: depthsFromSensor.depthMatrix.length,
      physics: new NeverScrollableScrollPhysics(),
      primary: true,
      children: List.generate(depthsFromSensor.totalSize, (index){
        return buildGridTile(depthsFromSensor.getDepth(index),
            depthsFromSensor.depthMatrix[0].length,
            depthsFromSensor.depthMatrix.length,
            context);
      }),
    );
  }

}