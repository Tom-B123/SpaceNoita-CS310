#ifndef QUAD_TREE
#define QUAD_TREE

#include "app.h"
#include <cstdlib>
class QuadTree {
    private:
        int width;
        int height;
        int* tree;
        bool* full;

        void add_to_tree(int val,int x,int y) {
            for (int level = 1; level <= log2(max(width,height)); level++) {
                int l_x = x / (width / (pow(2,level)));
                int l_y = x / (width / (pow(2,level)));
                std::cout << val << ": " << x << "," << y << " = " << l_x << "," << l_y << std::endl;
                std::cout << "get index to write this to: we want to split cells to reach the lowest level, without makeing unecessary splits. E.g. for 0,0 we don't need to split the TR, BL or BR quadrants, just TL"<< std::endl;

            }
        }
        /**
         *  Takes data and converts it into a tree.
         */
        int* make_tree(int* data) {
            // Empty tree, uses 4x the space to store up to log n levels
            int* buf = new int[width * height * 4];
            int* full = new int[width * height * 4];
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int val = data[y * width + x];
                    if (val > 0) {
                        add_to_tree(data[y * width + x],x,y);
                    }
                }
            }
            return buf;
        }
    public:
        QuadTree(int n_width, int n_height, int* data) : width(n_width), height(n_height) {
            tree = make_tree(data);
        }
        /**
         *  Data is stored as a char array representing 2d data.
         *  If data is > 0: Set all tree level that include x,y to max(val,data[x,y]).
         *  We should get the maximal value in pos 0, then in 1,2,3,4 get the maximal value of the 4 quadrants, etc.
         */

        void read_tree(int width, int height) {

        }
};

#endif
