import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'grid_display.dart';
import 'model/depths_from_sensor.dart';
import 'package:provider/provider.dart';

class DepthMapGrid extends StatefulWidget{
  final BluetoothDevice _device;

  const DepthMapGrid(this._device);

  @override
  State<StatefulWidget> createState() => _DepthMapGridState(this._device);
}

class _DepthMapGridState extends State<DepthMapGrid>{
  static const int _TRAME_SIZE = 20;
  static const int _TRAME_ID = 0x11;
  BluetoothDevice _device;

  DepthsFromSensor _depthsFromSensor = new DepthsFromSensor();
  StreamSubscription _deviceStateSubscription;
  BluetoothDeviceState _deviceState = BluetoothDeviceState.disconnected;
  List<BluetoothService> _services = new List();
  BluetoothCharacteristic _characteristic;
  bool _notifyingCharacteristic = false;

  _DepthMapGridState(this._device);

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
          if(s.isNotEmpty && s[s.length-1].characteristics.isNotEmpty){
            setState(() {
              _characteristic = s[s.length-1].characteristics[0];
            });
            if(!_characteristic.isNotifying){
              _characteristic.setNotifyValue(true).then((notifyState){
                setState(() {
                  _notifyingCharacteristic = notifyState;
                });
                if(notifyState){
                  _characteristic.value.listen((value) {
                    if (value.length == _TRAME_SIZE) {
                      int idTrame = value[0];
                      int numSequence = value[1];
                      int sizeRow = value[2];
                      if (idTrame == _TRAME_ID) {
                        List<int> row = new List<int>();
                        for (int i = 0; i < sizeRow; ++i) {
                          row.add(value[3 + i]);
                        }
                        this._depthsFromSensor.updateDepthMatrix(numSequence, row);
                      }
                    }
                  });
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