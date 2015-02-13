#pragma once

#include "constants.h"
#include "matrix.h"
#include "EasyBMP.h"

#include <tuple>

Image load_image(const char *);
void save_image(const Image &, const char *);
