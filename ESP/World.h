#pragma once
#include <Windows.h>

struct Vector3
{
    float x, y, z;
};

struct view_matrix_t
{
    float matrix[16];
};

struct mat3x4
{
    float c[3][4];
};