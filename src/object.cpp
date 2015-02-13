#include "object.h"

using namespace std;

#include <tuple>

static inline long long
llpow(long long a, uint power)
{
    //this function isn't supposed to be used with big powers
    if (power == 0) {
        return 1;
    } else if (power == 1) {
        return a;
    } else {
        long long ans = a * a;
        for (uint i = 2; i < power; i++) {
            ans *= a;
        }
        return ans;
    }
}


Coord
Object::get_center_of_mass() const
{
    uint m_x = 0, m_y = 0;
    for (uint y = y_min; y < y_max; y++) {
        for (uint x = x_min; x < x_max; x++) {
            if (bin_img(y, x) == number) {
                m_x += x;
                m_y += y;
            }
        }
    }
    return make_tuple(m_x / area, m_y / area);
}

//discrete central moment (see cg2014_02_basicanalysis.pdf, p.79)
int
Object::moment(uint pow_i, uint pow_j) const
{
    long long m_x, m_y, ans = 0;
    tie(m_x, m_y) = get_center_of_mass();
    for (uint y = y_min; y < y_max; y++) {
        for (uint x = x_min; x < x_max; x++) {
            if (bin_img(y, x) == number) {
                ans += llpow(x - m_x, pow_i) * llpow(y - m_y, pow_j);
            }
        }
    }
    return ans;
}

double
Object::get_axis_of_inertia_angle() const
{
    return 0.5 * atan2(2 * moment(1, 1), moment(2, 0) - moment(0, 2));
}
    
double
Object::get_elongation() const
{
    //see cg2014_02_basicanalysis.pdf, p.80
    long long m1 = moment(2, 0) + moment(0, 2);
    long long m2 = moment(2, 0) - moment(0, 2);
    long long m3 = moment(1, 1);
    double m4 = sqrt(m2*m2 + 4 * m3*m3);
    return (m1 + m4) / (m1 - m4);
}

bool
Object::is_border(uint y, uint x) const
{
    if (!img.in_bounds(y - 1, x - 1) || !img.in_bounds(y + 1, x + 1)) {
        return true;
    } else {
        return (bin_img(y, x - 1) != number)
            || (bin_img(y, x + 1) != number)
            || (bin_img(y - 1, x) != number)
            || (bin_img(y + 1, x) != number);
    }
}

double
Object::get_perimeter() const
{
    uint perimeter = 0;
    for (uint y = y_min; y < y_max; y++) {
        for (uint x = x_min; x < x_max; x++) {
            if (bin_img(y, x) == number && is_border(y, x)) {
                perimeter++;
            }
        }
    }
    return perimeter;
}

double
Object::get_compactness() const
{
    double P = get_perimeter();
    return P * P / area;
}

bool
Object::in_object(int i, int j) const
{
    return img.in_bounds(i, j) && (bin_img(i, j) == number);
}

uint
Object::green_c_near(uint y, uint x) const
{
    int i0 = y, j0 = x;
    uint r, g, b, ans = 0;
    const int radius = 5;  //radius of neighbourhood
    for (int i = i0 - radius; i <= i0 + radius; i++) {
        for (int j = j0 - radius; j <= j0 + radius; j++) {
            if (in_object(i, j)) {
                tie(r, g, b) = img(i, j);
                if (g > b) {
                    //green > blue => it's more or less green point
                    //it's the best implementation for white and red arrows
                    ans++;
                }
            }
        }
    }
    return ans;
}

//public:

Object::Object(const Image &_img, const BinImage &_bin_img, uint _number)
    : img(_img), bin_img(_bin_img), number(_number), area(0),
    x_max(0), y_max(0), x_min(bin_img.n_cols), y_min(bin_img.n_rows)
{}

bool
Object::is_noise() const
{
    return (area < min_area);
}

bool
Object::is_arrow() const
{
    auto elong = get_elongation();
    auto compact = get_compactness();
    return (elong_min_arrow < elong) && (elong < elong_max_arrow)
        && (compact_min_arrow < compact) && (compact < compact_max_arrow);
}

bool
Object::is_red() const
{
    uint r_sum = 0, g_sum = 0, b_sum = 0, r, g, b;
    for (uint y = y_min; y < y_max; y++) {
        for (uint x = x_min; x < x_max; x++) {
            if (bin_img(y, x) == number) {
                tie(r, g, b) = img(y, x);
                r_sum += r;
                g_sum += g;
                b_sum += b;
            }
        }
    }
    return (r_sum > g_sum + b_sum);         //red enough
}

bool
Object::is_white() const
{
    uint white_c = 0, non_white_c = 0, r, g, b;
    const uint col_treshold = 200, max_col_diff = 10; 
    for (uint y = y_min; y < y_max; y++) {
        for (uint x = x_min; x < x_max; x++) {
            if (bin_img(y, x) == number) {
                tie(r, g, b) = img(y, x);
                if (r > col_treshold && g > col_treshold && b > col_treshold
                    && (abs(r - g) < max_col_diff)
                    && (abs(g - b) < max_col_diff)
                    && (abs(b - r) < max_col_diff))
                {
                    white_c++;
                } else {
                    non_white_c++;
                }
            }
        }
    }
    //there should be at least twice as much white as non-white:
    return (white_c > non_white_c);
}
    
Rect
Object::get_borders() const
{
    return make_tuple(x_min, y_min, x_max - x_min + 1, y_max - y_min + 1);
}

void
Object::draw_borders()
{
    uint x, y = y_min;
    for (x = x_min; x < x_max; x++) {
        img(y, x) = red;
    }
    y = y_max - 1;
    for (x = x_min; x < x_max; x++) {
        img(y, x) = red;
    }
    x = x_min;
    for (y = y_min; y < y_max; y++) {
        img(y, x) = red;
    }
    x = x_max - 1;
    for (y = y_min; y < y_max; y++) {
        img(y, x) = red;
    }
}

void
Object::add_point(uint y, uint x)
{
    area++;
    if (x < x_min) {
        x_min = x;
    }
    if (x >= x_max) {
        x_max = x + 1;
    }
    if (y < y_min) {
        y_min = y;
    }
    if (y >= y_max) {
        y_max = y + 1;
    }
    bin_img(y, x) = number;
}
