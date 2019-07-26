import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'grid_display.dart';
import 'model/depths_from_sensor.dart';
import 'package:provider/provider.dart';
//TODO : secure setState to check if widget is still mounted so we don't call setState after a dispose
class DepthMapGrid extends StatefulWidget{
  final BluetoothDevice _device;

  const DepthMapGrid(this._device);

  @override
  State<StatefulWidget> createState() => _DepthMapGridState(this._device);
}

class _DepthMapGridState extends State<DepthMapGrid>{
  static const int _TRAME_ID_EVO64 = 0x11;
  static const int _TRAME_ID_VL53L1 = 0x50;
  static const String _SERVICE_UUID = "838f7fdd-4c42-405f-b8d4-83a698cce2e";
  static const String _CHARACT_SENSOR_STREAM_UUID = "a864bb58-1b21-4b89-8f5a-6947341abbf0";
  static const String _SERVICE_CONTROL_UUID = "00ff0000-fd7a-4c87-6373-712060e11c1e";
  static const String _CHARACT_COMMAND_UUID = "00ff0001-fd7a-4c87-6373-712060e11c1e";

  BluetoothDevice _device;

  DepthsFromSensor _depthsFromSensor = new DepthsFromSensor();
  StreamSubscription _deviceStateSubscription;
  BluetoothDeviceState _deviceState = BluetoothDeviceState.disconnected;
  List<BluetoothService> _services = new List();
  BluetoothCharacteristic _characteristic;
  bool _notifyingCharacteristic = false;

  _DepthMapGridState(this._device);

  void sendInitCommands(BluetoothCharacteristic c) async{
    //await c.write([0xBB, 0xB8, 0x01]); //start trip, see if needed
    await c.write([0xBB, 0xDF, 0x50]); //start stream for sensor of id 0x50
  }

  @override
  void initState() {
    _deviceStateSubscription = _device.state.listen((s) {
      setState(() {
        _deviceState = s;
      });
      if (s == BluetoothDeviceState.connected) {
        _device.discoverServices().then((s) {
          setState(() {
            _services = s;
          });
          //SERVICES
          if(s.isNotEmpty){
            //SEND INIT COMMANDS TO START THE STREAM
            BluetoothService serviceControlGlasses;
            BluetoothCharacteristic charactControl;
            for(BluetoothService serviceControl in s){
              if(serviceControl.uuid.toString() == _SERVICE_CONTROL_UUID){
                serviceControlGlasses = serviceControl;
                break;
              }
            }
            if(serviceControlGlasses != null){
              for(BluetoothCharacteristic charactCmd in serviceControlGlasses.characteristics){
                if(charactCmd.uuid.toString() == _CHARACT_COMMAND_UUID){
                  charactControl = charactCmd;
                  break;
                }
              }
            }
            if(charactControl != null){
              sendInitCommands(charactControl);
            }
            //END : SEND INIT COMMANDS TO START THE STREAM
            //FIND THE SENSOR STREAMING CHARACTERISTIC
            BluetoothService serviceGlasses;
            BluetoothCharacteristic charactSensorStream;
            for(BluetoothService service in s){
              if(service.uuid.toString() == _SERVICE_UUID){
                serviceGlasses = service;
                break;
              }
            }
            if(serviceGlasses != null){
              for(BluetoothCharacteristic charact in serviceGlasses.characteristics){
                if(charact.uuid.toString() == _CHARACT_SENSOR_STREAM_UUID){
                  charactSensorStream = charact;
                  break;
                }
              }
            }
            //END : FIND THE SENSOR STREAMING CHARACTERISTIC
            setState(() {
              _characteristic = (charactSensorStream != null) ? charactSensorStream : s[s.length-1].characteristics[0]; //compatibility with previous test module (should do smthing else if the characteristic is null)
            });
            if(!_characteristic.isNotifying){
              _characteristic.setNotifyValue(true).then((notifyState){
                setState(() {
                  _notifyingCharacteristic = notifyState;
                });
                if(notifyState){
                  //SUBSCRIBE TO THE SENSOR STREAMING CHARACTERISTIC
                  _characteristic.value.listen((value) {
                    if (value.length > 0) {
                      int idTrame = value[0];
                      switch(idTrame){
                        case _TRAME_ID_EVO64:
                          int numSequence = value[1];
                          int sizeRow = value[2];

                          List<int> row = new List<int>();
                          for (int i = 0; i < sizeRow; ++i) {
                            row.add(value[3 + i]);
                          }

                          this._depthsFromSensor.updateDepthMatrix(numSequence, row);
                          break;

                        case _TRAME_ID_VL53L1:
                          //int sizeRow = value[1]; can sqrt this size but we now in that case it's 16 so a 4*4 matrix
                          List<List<int>> newMatrix = new List<List<int>>();
                          newMatrix.add(new List<int>()); //row 0
                          newMatrix.add(new List<int>()); //row 1
                          newMatrix.add(new List<int>()); //row 2
                          newMatrix.add(new List<int>()); //row 3
                          newMatrix.add(new List<int>()); //row 4
                          newMatrix.add(new List<int>()); //row 5
                          newMatrix.add(new List<int>()); //row 6
                          newMatrix.add(new List<int>()); //row 7

                          //interpoler la matrice 4*4 en 8*8
                          for( int i = 0; i < 4; ++i){
                            for( int j = 0; j < 4; ++j){
                              newMatrix[i*2].add(value[(i*4)+j+2]);
                              newMatrix[i*2].add(value[(i*4)+j+2]);
                              newMatrix[(i*2)+1].add(value[(i*4)+j+2]);
                              newMatrix[(i*2)+1].add(value[(i*4)+j+2]);
                            }
                          }

                          this._depthsFromSensor.fullUpdateDepthMatrix(newMatrix);
                          break;

                        default:
                          break;
                      }
                    }
                  });
                  //END : SUBSCRIBE TO THE SENSOR STREAMING CHARACTERISTIC
                }
              });
            }
          }
        });
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    if(_deviceState != BluetoothDeviceState.connected || _notifyingCharacteristic == false){
      return Center(
        child: Text("Please wait for the connexion"),
      );
    }else{
      return Center(
        child: ChangeNotifierProvider<DepthsFromSensor>.value(
            notifier: this._depthsFromSensor,
            child: new GridDisplay()
        ),
      );
    }
  }

}