#pragma once

#include "matrix.h"

#include <tuple>

typedef Matrix<std::tuple<uint, uint, uint>> Image;     // <R, G, B>
typedef std::tuple<uint, uint> Coord;
typedef std::tuple<uint, uint, uint> Point;       // <R, G, B>
typedef Matrix<uint> BinImage;
typedef std::tuple<uint, uint, uint, uint> Rect;  //<left_up_x, left_up_y_, len_x, len_y>

const uint col_max = 255;   //maximum value of colour (1 byte for each colour)
const Point white = std::make_tuple(col_max, col_max, col_max);
const Point black = std::make_tuple(0, 0, 0);
const Point green = std::make_tuple(0, col_max, 0);
const Point red = std::make_tuple(col_max, 0, 0);
