#pragma once

#include "constants.h"
#include "matrix.h"

BinImage binarize(const Image &);
Image debinarize(const BinImage &);
Image noise_reduction(const Image &);
