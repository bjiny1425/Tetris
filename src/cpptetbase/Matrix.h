#pragma once
#include <iostream>
#include <cstdlib>

using namespace std;

class Matrix {
private:
  static int nAlloc;
  static int nFree;
  int dy;
  int dx;
  int **array;
  void alloc(int cy, int cx);
public:
  static int get_nAlloc();
  static int get_nFree();
  int get_dy();
  int get_dx();
  int** get_array();
  Matrix();
  Matrix(int cy, int cx);
  Matrix(const Matrix *obj);
  Matrix(const Matrix &obj);
  Matrix(int *arr, int col, int row);
  ~Matrix();
  Matrix *clip(int top, int left, int bottom, int right);
  void paste(const Matrix *obj, int top, int left);
  Matrix *add(const Matrix *obj);
  //실습
  friend const Matrix operator+(const Matrix& m1, const Matrix& m2);
  //Matrix operator+(const Matrix& m2) const; //콜링오브젝트의 변수들을 변경하지 않겠다

  int sum();
  void mulc(int coef);
  Matrix *int2bool();
  bool anyGreaterThan(int val);
  void print();
  friend ostream& operator<<(ostream& out, const Matrix& obj);
  Matrix& operator=(const Matrix& obj);
};
