#include <iostream>
#include <fstream>
#include <map>
#include <list>

#include "HouseSet.h"

using namespace std;

/**
* Constructor
*
* reads input file, and generates tree of houses sorted by type and then by width
*
* filename is input filename
*/
HouseSet::HouseSet(string const & filename)
{
    // allocate house map
    for(int t = -2; t <= 4; t++){
        list<houseType*>* houseListbyType = new list<houseType*>;
        houseTypeTree.insert(make_pair(t, houseListbyType));
    }

    ifstream fp;
    try{
        fp.open(filename);
        bool firstLine = true;
        // loop through lines
        while(!fp.eof()){
            string line;
            fp >> line;
            // skip first line (top row)
            if(firstLine || !line.compare("")){
                firstLine = false;
                continue;
            }
            string comma = ",";

            size_t pos = 0;
            int count = 0;
            string val;
            int32_t values[INPUT_COL_NUM];
            // loop through comma separated substring and get each value of it
            while ((pos = line.find(comma)) != string::npos) {
                val = line.substr(0, pos);
                // set the value in the array
                values[count] = (int32_t) stoi(val);
                line.erase(0, pos + 1);
                count++;
            }
            // get final substring
            values[count] = (int32_t) stoi(line);

            // make new housetype
            houseType* house = new houseType;
            
            // set main values
            house->ID = values[0];
            house->type = values[1];
            house->width = values[2];
            // set copy point values
            house->cpy_pt_x = values[3];
            house->cpy_pt_y = values[4];
            house->cpy_pt_z = values[5];
            // set pos 1 values
            house->pos1_x = values[6];
            house->pos1_y = values[7];
            house->pos1_z = values[8];
            // set pos 2 values
            house->pos2_x = values[9];
            house->pos2_y = values[10];
            house->pos2_z = values[11];

            // map it to the houseID map
            housesByID.insert(make_pair(values[0], house));

            // get list that house needs to go into
            list<houseType*>* houseList = houseTypeTree[house->type];
            list<houseType*>::iterator it;
            // simply add it if the list is empty
            if(houseList->size() == 0){
                houseList->push_back(house);
            }
            // otherwise iterate to add it
            else{
                it = houseList->begin();
                while(1){
                    // if iterator hits end of list, add it there
                    if(it == houseList->end()){
                        houseList->push_back(house);
                        break;
                    }
                    houseType* cur_house = *it;
                    // if the new house's width is less or equal to the current one, insert it before
                    if(cur_house->width >= house->width){
                        houseList->insert(it, house);
                        break;
                    }
                    ++it;
                }
            }
        }
    }
    catch (const ifstream::failure& e) {
        cout << "ERROR: could not open or read house data input file" << endl;
    }
}

/**
* pickRandHouseByWidth
*
* picks a random house with the given type that has a width that is in [minWidth, maxWidth]
* if minWidth = maxWidth, it picks a random house with exactly that width
* type - ID of type of house to be picked:
* -1 = unassigned, -2 = house between corners, 0 = no neighbors, 1 = "i", 2 = "L", 3 = "T", 4 = "+"
* minWidth - min width of house to be picked
* maxWidth - max width of house to be picked
*
* return - pointer to the house that was picked, returns null if inputs bad or if no house found
*
*/
HouseSet::houseType* HouseSet::pickRandHouseByWidth(int32_t typeID, int32_t minWidth, int32_t maxWidth)
{
    houseType* retVal = NULL;
    // check input for range
    if(typeID < -2 || typeID > 4 || maxWidth < minWidth || minWidth <= 0 || maxWidth <=0){
        cout << "ERROR: input into pickRandHouseByWidth is invalid" << endl;
        return retVal;
    }
    // get the list of houses with the type
    list<houseType*>* houseList = houseTypeTree[typeID];

    vector<houseType*> houseVec;
    // get all houses from list that are within the correct range
    for(auto house: *houseList){
        // break loop if hit house that has a width larger than the max allowed
        if(house->width > maxWidth){
            break;
        }
        // otherwise add it to the vector if its width is larger or equal to min
        else if(house->width >= minWidth){
            houseVec.push_back(house);
        }
    }
    size_t size = houseVec.size();
    if(size == 0){
        cout << "ERROR: attempted to pick house of width "<< minWidth <<" which is non-existent" << endl;
        return retVal;
    }
    // generate a random index from 0 - max size of vector
    int32_t index = rand() % size;
    // get ID of chosen house
    retVal = houseVec[index];

    return retVal;
}

/**
* printTree
*
* used for testing tree
*/
void HouseSet::printTree()
{
    for(int t = -2; t <= 4; t++){
        list<houseType*>* houseList = houseTypeTree[t];
        cout<<"Size of list for type "<<t<<": "<<houseList->size()<<endl;
        // print width of each house
        for(auto house: *houseList){
            cout<<house->ID<<" "<<house->width<<endl;
        }
    }
}

/**
* Destructor
*
* frees memory inside each list in the map, then frees all lists
*/
HouseSet::~HouseSet()
{
    // free each list for each possible type
    for(int t = -2; t <= 4; t++){
        list<houseType*>* houseList = houseTypeTree[t];
        // free houses in list first
        for(auto house: *houseList){
            delete house;
        }
        // finally free list
        delete houseList;
    }
}