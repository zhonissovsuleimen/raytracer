#include "matrix.h"

Matrix3D::Matrix3D() {
  data = new float[9];
  for (int i = 0; i < 9; i++) {
    data[i] = 0;
  }
}

Matrix3D::Matrix3D(
  float a, float b, float c, 
  float d, float e, float f, 
  float g, float h, float i) {
  data = new float[9];
  data[0] = a;
  data[1] = b;
  data[2] = c;
  data[3] = d;
  data[4] = e;
  data[5] = f;
  data[6] = g;
  data[7] = h;
  data[8] = i;
}

Matrix3D::~Matrix3D() {
  delete[] data;
}

Matrix3D Matrix3D::operator+=(Matrix3D &m) {
  for (int i = 0; i < 9; i++) {
    data[i] += m.data[i];
  }
  return *this;
}

Matrix3D Matrix3D::operator-=(Matrix3D &m) {
  for (int i = 0; i < 9; i++) {
    data[i] -= m.data[i];
  }
  return *this;
}

Matrix3D Matrix3D::operator*=(Matrix3D &m) {
  float* temp = new float[9];
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        temp[i * 3 + j] += data[i * 3 + k] * m.data[k * 3 + j];
      }
    }
  }
  for (int i = 0; i < 9; i++) {
    data[i] = temp[i];
  }
  delete[] temp;
  return *this;
}

Matrix3D Matrix3D::operator*=(float f) {
  for (int i = 0; i < 9; i++) {
    data[i] *= f;
  }
  return *this;
}

Matrix3D Matrix3D::operator/=(float f) {
  for (int i = 0; i < 9; i++) {
    data[i] /= f;
  }
  return *this;
}

float Matrix3D::deteminant() {
  return data[0] * (data[4] * data[8] - data[5] * data[7]) -
         data[1] * (data[3] * data[8] - data[5] * data[6]) +
         data[2] * (data[3] * data[7] - data[4] * data[6]);
}

Matrix3D Matrix3D::transpose(){
  float* temp = new float[9];
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      temp[i * 3 + j] = data[j * 3 + i];
    }
  }
  for (int i = 0; i < 9; i++) {
    data[i] = temp[i];
  }
  delete[] temp;
  return *this;
}