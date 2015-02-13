#include "preprocess.h"

using namespace std;

#include <tuple>
#include <cstring>


class Colours_check
{
    static const uint chart_len = col_max * 3; // RGB: 3 colour of depth col_max
    uint brightness_chart[chart_len];
    // brightness is implemented as sum of colour components
    
public:
    // Radius of neighbourhood, which is passed to that operator
    // Here we need every pixel only once, so radius is 0
    static const int radius = 0;
    
    Colours_check()
    {
        memset(brightness_chart, 0, sizeof(brightness_chart));
    }
    
    Point
    operator() (const Image &img)
    {
        uint r, g, b;
        tie(r, g, b) = img(0, 0);       //img consist of only 1 pixel
        brightness_chart[r + g + b]++;
        return img(0,0);
    }
    
    tuple<uint, bool>
    get_threshold() const
    {
        uint ind_max = 0, max = brightness_chart[ind_max];
        for (uint i = 1; i < chart_len; i++) {
            if (brightness_chart[i] > max) {
                ind_max = i;
                max = brightness_chart[ind_max];
            }
        }
        
        uint sum_left = 0, sum_right = 0;
        for (uint i = 0; i < ind_max; i++) {
            sum_left += brightness_chart[i];
        }
        for (uint i = ind_max; i < chart_len; i++) {
            sum_right += brightness_chart[i];
        }
        
        const double p = 0.05;  //portion to make neigbourhood of max on chart
        const uint shift = 24;  //magic constant to shift from max (useful when max is on border)
        bool bg_on_right;
        uint T, ind_p, sum_p = 0;
        if (sum_left > sum_right) {
            //objects are on left from max (background):
            bg_on_right = true;
            for (ind_p = chart_len - 1; ind_p > ind_max; ind_p--) {
                sum_p += brightness_chart[ind_p];
                if (double(sum_p) / sum_right >= p) {
                    break;
                }
            }
            T = ind_max - (ind_p - ind_max);
            if (ind_max == chart_len) {
                T = ind_max - shift;
            }
        } else {
            //objects are on right from max (background):
            bg_on_right = false;
            for (ind_p = 0; ind_p < ind_max; ind_p++) {
                sum_p += brightness_chart[ind_p];
                if (double(sum_p) / sum_left >= p) {
                    break;
                }
            }
            T = ind_max + (ind_max - ind_p);
            if (ind_max == 0) {
                T = ind_max + shift;
            }
        }
        
        return make_tuple(T, bg_on_right);
    }
};

class Binarization
{
    const uint thresh_val;
    const bool brighter_bg;     //flag: background is brighter than threshold
    
public:
    // Radius of neighbourhoud, which is passed to that operator
    // Here we need every pixel only once, so radius is 0
    static const int radius = 0;
    
    Binarization(tuple<uint, bool> threshold)
        : thresh_val(get<0>(threshold)), brighter_bg(get<1>(threshold))
    {}
    
    uint
    operator() (const Image &img) const
    {
        uint r, g, b;
        tie(r, g, b) = img(0, 0);       //img consist of only 1 pixel
        return
            (brighter_bg) ? (r + g + b < thresh_val) : (r + g + b > thresh_val);
    }
};
    
BinImage 
binarize(const Image &img)
{
    auto check_col = Colours_check();
    img.unary_map(check_col);  //checkig colours to find out threshold that divide background and objects
    return img.unary_map(Binarization(check_col.get_threshold()));
}

class Debinarization
{
public:
    // Radius of neighbourhoud, which is passed to that operator
    // Here we need every pixel only once, so radius is 0
    static const int radius = 0;
    
    Point
    operator() (const BinImage &bi) const
    {
        return bi(0,0) ? white : black;
    }
};

Image
debinarize(const BinImage &bin_img)
{
    return bin_img.unary_map(Debinarization());
}

//different preprocessing filters (see also matrix_example.cpp):

struct Median_filter
{
    // Radius of neighbourhoud, which is passed to that operator
    static const int radius = 1;
    
    Point
    operator() (const Image &img) const
    {
        uint r, g, b;
        for (uint i = 0; i < size; ++i) {
            for (uint j = 0; j < size; ++j) {
                tie(r, g, b) = img(i, j);
                r_neighbours[i * size + j] = r;
                g_neighbours[i * size + j] = g;
                b_neighbours[i * size + j] = b;
            }
        }
        sort(r_neighbours.begin(), r_neighbours.end());
        sort(g_neighbours.begin(), g_neighbours.end());
        sort(b_neighbours.begin(), b_neighbours.end());
        
        return make_tuple(r_neighbours[m], g_neighbours[m], b_neighbours[m]);
    }

private:
    uint size = 2 * radius + 1;
    mutable vector<uint> r_neighbours = vector<uint>(size * size);
    mutable vector<uint> g_neighbours = vector<uint>(size * size);
    mutable vector<uint> b_neighbours = vector<uint>(size * size);
    uint m = r_neighbours.size() / 2;    //length is same in all 3 vectors
};

Image
noise_reduction(const Image &img)
{
    //any preprocessing filters you like
    return img.unary_map(Median_filter());
}
