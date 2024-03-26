#pragma once

#include <vector>

class Matrix3D {
private:
    float* data;

public:
    Matrix3D();
    Matrix3D(float, float, float, float, float, float, float, float, float);
    ~Matrix3D();

    inline float operator()(int row, int col) { return data[row * 3 + col]; }

    Matrix3D operator+=(Matrix3D &m);
    Matrix3D operator-=(Matrix3D &m);
    Matrix3D operator*=(Matrix3D &m);
    Matrix3D operator*=(float f);
    Matrix3D operator/=(float f);

    float det();
    Matrix3D transpose();
};

Matrix3D operator+(Matrix3D &m1, Matrix3D &m2);
Matrix3D operator-(Matrix3D &m1, Matrix3D &m2);
Matrix3D operator*(Matrix3D &m1, Matrix3D &m2);
Matrix3D operator*(Matrix3D &m1, Matrix3D &m2);
Matrix3D operator*(Matrix3D &m, float f);
Matrix3D operator*(float f, Matrix3D &m);
Matrix3D operator/(Matrix3D &m, float f);
Matrix3D operator/(float f, Matrix3D &m);
