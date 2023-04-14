#include "Matrix.h"

int Matrix::nAlloc = 0;
int Matrix::nFree = 0;

int Matrix::get_nAlloc() { return nAlloc; }

int Matrix::get_nFree() { return nFree; }

int Matrix::get_dy() { return dy; } // 세로

int Matrix::get_dx() { return dx; } // 가로

int **Matrix::get_array() { return array; }

void Matrix::alloc(int cy, int cx) { // 행렬을 동적으로 할당
  if ((cy < 0) || (cx < 0)) return;
  dy = cy;
  dx = cx;
  array = new int*[dy]; // int형 이중 포인터를 dy만큼 동적 할당
  for (int y = 0; y < dy; y++)
    array[y] = new int[dx]; // dx만큼 동적 할당
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = 0; // 0으로 초기화
  
  nAlloc++; // 생성자 호출될 때 할당된 행렬 개수 기록
}

Matrix::Matrix() { alloc(0, 0); } // 행렬 생성자 (dy x dx)

Matrix::~Matrix() { // 행렬 소멸자
  for (int y = 0; y < dy; y++)
    delete array[y]; // 2차원 배열 해제
  delete array;

  nFree++; // 해제된 행렬 개수 기록
}

Matrix::Matrix(int cy, int cx) { // 생성자
  alloc(cy, cx); // 행렬의 크기 동적 할당
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = 0;
}

Matrix::Matrix(const Matrix *obj) { // 복사 생성자
  alloc(obj->dy, obj->dx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = obj->array[y][x]; // 현재 객체에 복사할 값 대입
}

Matrix::Matrix(const Matrix &obj) { // 복사 생성자
  alloc(obj.dy, obj.dx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = obj.array[y][x];
}

Matrix::Matrix(int *arr, int col, int row) {
  alloc(col, row);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = arr[y * dx + x];
}

Matrix *Matrix::clip(int top, int left, int bottom, int right) {
  int cy = bottom - top;
  int cx = right - left;
  Matrix *temp = new Matrix(cy, cx);
  for (int y = 0; y < cy; y++) {
    for (int x = 0; x < cx; x++) {
      if ((top + y >= 0) && (left + x >= 0) &&
	  (top + y < dy) && (left + x < dx))
	temp->array[y][x] = array[top + y][left + x];
      else {
	cerr << "invalid matrix range";
	return NULL;
      }
    }
  }
  return temp;
}

void Matrix::paste(const Matrix *obj, int top, int left) { // 다른 Matrix 객체복사
  for (int y = 0; y < obj->dy; y++)
    for (int x = 0; x < obj->dx; x++) {
      if ((top + y >= 0) && (left + x >= 0) &&
	  (top + y < dy) && (left + x < dx))
	array[y + top][x + left] = obj->array[y][x];
      else {
	cerr << "invalid matrix range";
	return;
      }
    }
}

Matrix *Matrix::add(const Matrix *obj) {
  if ((dx != obj->dx) || (dy != obj->dy)) return NULL;
  Matrix *temp = new Matrix(dy, dx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      temp->array[y][x] = array[y][x] + obj->array[y][x];
  return temp;
}

//실습
const Matrix operator+(const Matrix& m1, const Matrix& m2){
  if ((m1.dx != m2.dx) || (m1.dy!= m2.dy)) return Matrix();
  Matrix temp(m1.dy, m1.dx);
  for (int y = 0; y < m1.dy; y++)
    for (int x = 0; x < m1.dx; x++)
      temp.array[y][x] = m1.array[y][x] + m2. array[y][x];
  return temp;
}

// Matrix Matrix::operator+(const Matrix& m2) const { 
//   if ((dx != m2.dx) || (dy != m2.dy)) return Matrix();
//   Matrix temp(dy, dx);
//   for (int y = 0; y < dy; y++)
//     for (int x = 0; x < dx; x++)
//       temp.array[y][x] = array[y][x] + m2.array[y][x];
//   return temp;
// }

int Matrix::sum() {
  int total = 0;
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      total += array[y][x];
  return total;
}

void Matrix::mulc(int coef) {
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = coef * array[y][x];
}

Matrix *Matrix::int2bool() {
  Matrix *temp = new Matrix(dy, dx);
  int **t_array = temp->get_array();
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      t_array[y][x] = (array[y][x] != 0 ? 1 : 0);
  
  return temp;
}

bool Matrix::anyGreaterThan(int val) {
  for (int y = 0; y < dy; y++) {
    for (int x = 0; x < dx; x++) {
      if (array[y][x] > val)
	return true;
    }
  }
  return false;
}

void Matrix::print() {
  cout << "Matrix(" << dy << "," << dx << ")" << endl;
  for (int y = 0; y < dy; y++) {
    for (int x = 0; x < dx; x++)
      cout << array[y][x] << " ";
    cout << endl;
  }
}


ostream& operator<<(ostream& out, const Matrix& obj){
  out << "Matrix(" << obj.dy << "," << obj.dx << ")" << endl;
  for(int y = 0; y < obj.dy; y++){
    for(int x = 0; x < obj.dx; x++)
      out << obj.array[y][x] << " ";
    out << endl;
  }
  out << endl;
  return out;
}

Matrix& Matrix::operator=(const Matrix& obj)
{
  if (this == &obj) return *this;
  if ((dx != obj.dx) || (dy != obj.dy))
    alloc(obj.dy, obj.dx);

  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = obj.array[y][x];
  return *this;
}
