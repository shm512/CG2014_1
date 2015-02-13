#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <vector>
#include <tuple>
#include <queue>

using namespace std;

#include "constants.h"
#include "io.h"
#include "matrix.h"
#include "preprocess.h"
#include "segmentation.h"
#include "object.h"


static inline Object
find_red_arrow(const vector<Object> &objects)
{
    for (auto &obj : objects) {
        if (!obj.is_noise() && obj.is_arrow() && obj.is_red()) {
            return obj;
        }
    }
    //we should never be here:
    return *objects.begin();
}

//transform from signed polar coordinates to Cartesian coordinates
//with shift of coordinate system origin to (x0, y0)
//answer, r, x0 and y0 (but not phi!) supposed to be discrete
//sign of r means direction
static inline Coord
polar2cartesian_discr(int r, double phi, int x0, int y0)
{
    uint x = x0 + int(round(r * cos(phi)));
    uint y = y0 + int(round(r * sin(phi)));
    return make_tuple(x, y);
}

    
//this function finds out the direction that arrow is showing
//and proceeding there till the end of the arrow
//return value: axis angle, direction sign (1 or -1) and first point out of arrow
//return value order: <angle, dir_sign, first_out_x, first_out_y>
// N.B. THIS FUNCTION WORKS PROPEPLY ONLY WHEN OBJECT IS ARROW WITH GREEN POINT!
tuple<double, int, uint, uint>
Object::get_arrow_direction()
{
    double r = 0, phi = get_axis_of_inertia_angle();
    uint x0, y0;
    tie(x0, y0) = get_center_of_mass();
    //checking positive direction:
    uint x_pos = x0, y_pos = y0;
    unsigned long long green_c_pos = 0;
    for (r = 0; in_object(y_pos, x_pos); r++) {
        green_c_pos += green_c_near(y_pos, x_pos);   //count green pixels nearby
        tie(x_pos, y_pos) = polar2cartesian_discr(r, phi, x0, y0);
    }
    //now checking negative direction:
    uint x_neg = x0, y_neg = y0;
    unsigned long long green_c_neg = 0;
    for (r = 0; in_object(y_neg, x_neg); r--) {
        green_c_neg += green_c_near(y_neg, x_neg);   //count green pixels nearby
        tie(x_neg, y_neg) = polar2cartesian_discr(r, phi, x0, y0);
    }
    //we need direction with more green:
    if (green_c_pos > green_c_neg) {
        return make_tuple(phi, 1, x_pos, y_pos);
    } else {
        return make_tuple(phi, -1, x_neg, y_neg);
    }
}

tuple<vector<Rect>, Image>
find_treasure(const Image &in)
{
    Image img = noise_reduction(in);
    BinImage bin_img = binarize(img);
    vector<Object> objects = image_segmentation(img, bin_img);
    
    vector<Rect> path;
    int dir_sign;
    double phi;
    uint x0, y0;
    //going by arrows, starting at the red one:
    Object cur_obj = find_red_arrow(objects);
    do {
        path.push_back(cur_obj.get_borders());
        tie(phi, dir_sign, x0, y0) = cur_obj.get_arrow_direction();
        uint x = x0, y = y0;
        int r = 0;
        //indices in objects = object.number - 2 = bin_img(point_of_object) - 2
        //(0 and 1 for background and unmarked objects respectively)
        while (!bin_img(y, x) || objects[bin_img(y, x) - 2].is_noise()) {
            tie(x, y) = polar2cartesian_discr(r, phi, x0, y0);
            img(y, x) = green;
            r += dir_sign;
        }
        cur_obj = objects[bin_img(y, x) - 2];
    } while (cur_obj.is_arrow() && cur_obj.is_white());
    //finally, cur_obj is neither noise nor arrow => it is a treasure
    path.push_back(cur_obj.get_borders());
    cur_obj.draw_borders();
    return make_tuple(path, img);
}

int
main(int argc, char *argv[])
{
    if (argc != 4) {
        cout << "Usage: " << endl << argv[0]
             << " <in_image.bmp> <out_image.bmp> <out_path.txt>" << endl;
        return 0;
    }

    try {
        Image src_image = load_image(argv[1]);
        ofstream fout(argv[3]);

        vector<Rect> path;
        Image dst_image;
        tie(path, dst_image) = find_treasure(src_image);
        save_image(dst_image, argv[2]);

        uint x, y, width, height;
        for (const auto &obj : path) {
            tie(x, y, width, height) = obj;
            fout << x << " " << y << " " << width << " " << height << endl;
        }

    } catch (const string &s) {
        cerr << "Error: " << s << endl;
        return 1;
    }
}
