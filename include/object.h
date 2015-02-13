#pragma once

#include "constants.h"

#include <tuple>

class Object
{
    Image img;
    BinImage bin_img;
    uint number, area;                   //identifying number; area in points 
    uint x_max, y_max, x_min, y_min;     //maximums out of boundaries!
    static const uint min_area = 24;     //any object with area less is noise
    constexpr static const double elong_min_arrow = 2.7, elong_max_arrow = 5.0;
    constexpr static const double compact_min_arrow = 12.0, compact_max_arrow = 20.0;
    //object with elongation in these borders seem to be arrow
    
    Coord get_center_of_mass() const;
    
    //discrete central moment
    int moment(uint pow_i, uint pow_j) const;
    
    double get_axis_of_inertia_angle() const;
        
    double get_perimeter() const;
    
    double get_elongation() const;
    
    double get_compactness() const;
    
    bool in_object(int i, int j) const;
    
    bool is_border(uint i, uint j) const;
    
    //count pixels with predominating green component in neighbourhood
    uint green_c_near(uint i, uint j) const;
    
public:
    Object(const Image &, const BinImage &, uint);
    
    bool is_noise() const;
    
    bool is_arrow() const;
    
    bool is_red() const;
    
    bool is_white() const;
    
    Rect get_borders() const;
    
    void draw_borders();
    
    void add_point(uint i, uint j);
    
    //this function finds out the direction that arrow is showing
    //and proceeding there till the end of the arrow
    //return value: axis angle, direction sign (1 or -1) and first point out of arrow
    //return value order: <angle, dir_sign, first_out_x, first_out_y>
    // N.B. implementation in main.cpp
    std::tuple<double, int, uint, uint> get_arrow_direction();
};
