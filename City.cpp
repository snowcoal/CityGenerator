#include <iostream>
#include <cmath>

#include "City.h"
#include "PNGimage/PNG.h"
#include "PNGimage/HSLAPixel.h"

using namespace PNGimage;
using namespace std;

/**
* Constructor
*
* input_img - input PNG into constructor
* grid_box_width - width of grid boxes
*/
City::City(PNG input, int32_t grid_box_width)
{
    // // create test pixels
    // HSLAPixel white_px(0.0,0.0,1.0);
    // HSLAPixel black_px(0.0,0.0,0.0);

    // init basic private variables
    cityWidth = input.width();
    cityLength = input.height();
    input_img = &input;
    // *** 13 for now *** (will come from config file eventually)
    grid_cell_size = grid_box_width;
    gridWidth = cityWidth/(grid_cell_size + GRID_SPACE) + 2;
    gridLength = cityLength/(grid_cell_size + GRID_SPACE) + 2;

    // get area of cell and space
    uint32_t grid_cell_area = pow(grid_cell_size,2);

    // init grid cells
    for(int i = 0; i < gridLength; i++){
        // create empty row vector
        vector<gridCell*> row(gridWidth);
        for(int j = 0; j < gridWidth; j++){
            gridCell* cell = new gridCell;
            row[j] = cell;

            // // find if its a space or not
            // if(i%(grid_cell_size + GRID_SPACE) == 0 && j%(grid_cell_size + GRID_SPACE) == 0){
            //     cell->isSpace = 1;
            // }

            // the corner position needs to be -squarew,-squarew
            cell->pos_x = j * (grid_cell_size + GRID_SPACE) - (grid_cell_size);
            cell->pos_z = i * (grid_cell_size + GRID_SPACE) - (grid_cell_size);
            // pos_y TBH
            cell->isRoad = 0;

            // find if the cell needs to be in city or not
            int32_t count = 0;
            // loop for each pixel within the current grid cell
            for(int k = cell->pos_x; k < cell->pos_x + grid_cell_size; k++){
                for(int l = cell->pos_z; l < cell->pos_z + grid_cell_size; l++){
                    // check if checked pixel is in bounds
                    if(k>=0 && l>=0 && k<cityWidth && l<cityLength){
                        // get the current pixel
                        HSLAPixel & pixel = input.getPixel(k, l);
                        // check whether the luminance value is equal to the white pixel
                        if((int32_t)pixel.l == WHITE_PX_LUM){
                            count++;
                        }
                    }
                }
            }

            // check whether enough pixels are white
            if((double)count / (double)grid_cell_area > PX_PERCENTAGE){
                cell->inCity = 1;
                // std::cout<<"success"<<std::endl;
            }
            else{
                cell->inCity = 0;
            }
        }
        // add row to grid
        grid.push_back(row);
    }

    PrintGrid();

    // put road along outside of city

    // each list cell will have coords same as image pixel

    // if cell is in image it will have inCity = 1 else 0

}

/**
* PrintGrid
*
* prints out the grid of the city
*
*/
void City::PrintGrid(){
    HSLAPixel red_px(100,0.5,0.5);
    PNG input_cpy = (*input_img);

    for(int i = 0; i < gridLength; i++){
        for(int j = 0; j < gridWidth; j++){
            gridCell* cell = grid[i][j];
            // check if the cells in the city
            if(cell->inCity){
                for(int k = cell->pos_x; k < cell->pos_x + grid_cell_size; k++){
                    for(int l = cell->pos_z; l < cell->pos_z + grid_cell_size; l++){
                        HSLAPixel & pixel = input_cpy.getPixel(k, l);
                        pixel = red_px;
                    }
                }
            }
        }
    }
    input_cpy.writeToFile("gridOutput.png");
}

/**
* GenerateMaze
*
* generates maze inside city, sets
*
*/
void City::GenerateMaze(){
    // generate a maze on all points that are inCity
}

/**
* Destructor
*
* frees gridcells
*
*/
City::~City(){
    for(int i = 0; i < gridLength; i++){
        for(int j = 0; j < gridWidth; j++){
            free(grid[i][j]);
        }
    }
}