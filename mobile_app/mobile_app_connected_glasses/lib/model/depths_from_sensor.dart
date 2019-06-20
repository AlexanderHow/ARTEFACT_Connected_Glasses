import 'package:flutter/material.dart';

/// Parse les trames input provenant du capteur TeraRanger EVO 64px en une matrice représentant la carte de profondeur
class DepthsFromSensor with ChangeNotifier{
  List<List<int>> _depthMatrix;

  DepthsFromSensor(){
    this._depthMatrix = new List<List<int>>();
    this._init8x8();
  }

  List<List<int>> get depthMatrix => _depthMatrix;

  void updateDepthMatrix(int seqIndex, List<int> row){
    if(seqIndex > this._depthMatrix.length - 1){
      for(int i = this._depthMatrix.length - 1; i < seqIndex; ++i){
        this._depthMatrix.add(new List<int>());
      }
      this._depthMatrix.add(row);
    }else{
      this._depthMatrix[seqIndex] = new List<int>();
      for(int j = 0; j < row.length; ++j){
        this._depthMatrix[seqIndex].add(row[j]);
      }
    }
    notifyListeners();
  }

  void _init8x8(){
    for(int i = 0; i < 8; ++i){
      this._depthMatrix.add(new List<int>());
      for(int j = 0; j < 8; ++j){
        this._depthMatrix[i].add(0);
      }
    }
  }
}