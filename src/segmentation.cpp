#include "segmentation.h"

using namespace std;

#include <queue>
#include <vector>

void
segmentation_BFS(
    queue<Coord> &unmarked_points,
    Object &segment,
    const BinImage &bin_img)
{
    int i0, j0, i_bound = bin_img.n_rows, j_bound = bin_img.n_cols;
    do {
        tie(i0, j0) = unmarked_points.front();
        for (int i = i0 - 1; i <= i0 + 1; i++) {
            if (i < 0 || i >= i_bound) {
                continue;
            }
            for (int j = j0 - 1; j <= j0 + 1; j++) {
                if (j < 0 || j >= j_bound) {
                    continue;
                }
                if (bin_img(i, j) == 1) {
                    segment.add_point(i, j);
                    unmarked_points.push(make_tuple(i, j));
                }
            }
        }
        unmarked_points.pop();
    } while (!unmarked_points.empty());
}

vector<Object>
image_segmentation(const Image &img, BinImage &bin_img)
{
    queue<Coord> unmarked_objects_points;
    vector<Object> marked_objects;
    uint cur_num = 1;         //number of last marked segment
    for (uint i = 0; i < bin_img.n_rows; i++) {
        for (uint j = 0; j < bin_img.n_cols; j++) {
            if (bin_img(i, j) == 1) {
                auto new_segment = Object(img, bin_img, ++cur_num);
                new_segment.add_point(i, j);
                unmarked_objects_points.push(make_tuple(i, j));
                segmentation_BFS(unmarked_objects_points, new_segment, bin_img);
                marked_objects.push_back(new_segment);
            }
        }
    }
    return marked_objects;
}
