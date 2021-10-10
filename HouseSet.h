#pragma once

#include <map>
#include <list>

using std::vector;
using std::string;
using std::list;
using std::map;

#define  INPUT_COL_NUM                  12


class HouseSet
{
    private:
        // house type defines positional data of original houses on actual map
        struct houseType{
            // ID of house (for mapping it in game)
            int32_t ID;
            // -1 = unassigned, -2 = house between corners, 0 = no neighbors, 1 = "i", 2 = "L", 3 = "T", 4 = "+"
            int32_t type;
            // width of house
            int32_t width;
            
            // copy from location positional data
            int32_t cpy_pt_x;
            int32_t cpy_pt_y;
            int32_t cpy_pt_z;
            
            // pos1 data (one point of the bounding box)
            int32_t pos1_x;
            int32_t pos1_y;
            int32_t pos1_z;
            
            // pos2 data (2nd point of the bounding box)
            int32_t pos2_x;
            int32_t pos2_y;
            int32_t pos2_z;
        };

        // tree of input houses sorted by type, and then by width
        // inputting an type gets a list that contains houses with that same type, and
        // each list is sorted smallest to largest based on width
        map<int32_t, list<houseType*>*> houseTypeTree;


    public:
        // Constructor
        HouseSet(string const & filename);
        // Destructor
        ~HouseSet();

        // // gets house list for input type
        // getHouseList(int32_t type)

        // tree testing method
        void printTree();

};