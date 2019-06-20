import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

import 'depth_map_grid.dart';

class DepthMap extends StatelessWidget {

  final BluetoothDevice _device;

  DepthMap(this._device);

  @override
  Widget build(BuildContext context) {
    this._device.discoverServices();
    StreamBuilder<List<BluetoothService>>(
      stream: this._device.services,
      initialData: [],
      builder: (c, snapshot) {
        return Scaffold(
          appBar: AppBar(
          title: Text(this._device.name),
          actions: <Widget>[
            StreamBuilder<BluetoothDeviceState>(
              stream: this._device.state,
              initialData: BluetoothDeviceState.connecting,
              builder: (c, snapshotState) {
                VoidCallback onPressed;
                String text;
                switch (snapshotState.data) {
                  case BluetoothDeviceState.connected:
                    onPressed = () => this._device.disconnect();
                    text = 'DISCONNECT';
                    break;
                  case BluetoothDeviceState.disconnected:
                    onPressed = () => this._device.connect();
                    text = 'CONNECT';
                    break;
                  default:
                    onPressed = null;
                    text = snapshotState.data.toString().substring(21).toUpperCase();
                    break;
                }
                return FlatButton(
                  onPressed: onPressed,
                  child: Text(
                  text,
                  style: Theme.of(context)
                      .primaryTextTheme
                      .button
                      .copyWith(color: Colors.white),
                ));
              },
            )
          ],
          ),
          body: DepthMapGrid(snapshot.data[0].characteristics[0]), //get the only service which have one characteristic
        );
      }
    );
  }


}