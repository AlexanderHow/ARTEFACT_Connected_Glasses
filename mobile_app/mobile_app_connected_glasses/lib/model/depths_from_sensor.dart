import 'dart:math';

import 'package:flutter/material.dart';

/// Parse les trames input provenant du capteur TeraRanger EVO 64px en une matrice représentant la carte de profondeur
class DepthsFromSensor with ChangeNotifier{
  List<List<int>> _depthMatrix;

  DepthsFromSensor(){
    this._depthMatrix = new List<List<int>>();
    this._init8x8();
  }

  List<List<int>> get depthMatrix => _depthMatrix;
  int get totalSize => _depthMatrix.length * _depthMatrix[0].length;

  ///Get la profondeur du index ième pixel dans la matrice
  int getDepth(int index){
    return this._depthMatrix[index ~/ this._depthMatrix[0].length][index % this._depthMatrix[0].length];
  }

  ///Mise a jour de la matrice de profondeur par ligne reçue
  void updateDepthMatrix(int seqIndex, List<int> row){
    print("UPDATING "+seqIndex.toString());
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

  ///Initie la matrice 8x8 de profondeur à 0
  void _init8x8(){
    for(int i = 0; i < 8; ++i){
      this._depthMatrix.add(new List<int>());
      for(int j = 0; j < 8; ++j){
        this._depthMatrix[i].add(255);
      }
    }
    this._depthMatrix[0][0]=0;
  }
}