// Flutter code sample for widgets.Listener.1

// This example makes a [Container] react to being entered by a mouse
// pointer, showing a count of the number of entries and exits.

import 'package:flutter/material.dart';

import 'package:flutter/gestures.dart';

void main() => runApp(MyApp());

/// This Widget is the main application widget.
class MyApp extends StatelessWidget {
  static const String _title = 'Flutter Code Sample';

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: _title,
      home: Scaffold(
        appBar: AppBar(title: Text(_title)),
        body: MyStatefulWidget(),
      ),
    );
  }
}

class MyStatefulWidget extends StatefulWidget {
  MyStatefulWidget({Key key}) : super(key: key);

  @override
  _MyStatefulWidgetState createState() => _MyStatefulWidgetState();
}

class _MyStatefulWidgetState extends State<MyStatefulWidget> {
  int _enterCounter = 0;
  int _exitCounter = 0;
  double x = 0.0;
  double y = 0.0;

  void _incrementCounter(PointerEnterEvent details) {
    setState(() {
      _enterCounter++;
    });
  }

  void _decrementCounter(PointerExitEvent details) {
    setState(() {
      _exitCounter++;
    });
  }

  void _updateLocation(PointerHoverEvent details) {
    setState(() {
      x = details.position.dx;
      y = details.position.dy;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Center(
      child: ConstrainedBox(
        constraints: BoxConstraints.tight(Size(300.0, 200.0)),
        child: Listener(
          onPointerEnter: _incrementCounter,
          onPointerHover: _updateLocation,
          onPointerExit: _decrementCounter,
          child: Container(
            color: Colors.lightBlueAccent,
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                Text('You have pointed at this box this many times:'),
                Text(
                  '$_enterCounter Entries\n$_exitCounter Exits',
                  style: Theme.of(context).textTheme.display1,
                ),
                Text(
                  'The cursor is here: (${x.toStringAsFixed(2)}, ${y.toStringAsFixed(2)})',
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
