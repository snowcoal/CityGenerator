#include <iostream>
#include <cmath>
#include <cstdlib>
#include <stack>

#include "City.h"
#include "PNGimage/PNG.h"
#include "PNGimage/HSLAPixel.h"

using namespace PNGimage;
using namespace std;

/*
* Constructor for no heightmap
*
* input_img - input PNG into constructor
* grid_box_width - width of grid boxes
*/
City::City(PNG* input, int32_t grid_box_width)
{
    heightmap_img = NULL;
    init(input, grid_box_width);
}

/*
* Constructor for heightmap
*
* input_img - input PNG into constructor
* heightmap - heightmap input
* grid_box_width - width of grid boxes
*/
City::City(PNG* input, PNG* heightmap, int32_t grid_box_width)
{
    heightmap_img = heightmap;
    init(input, grid_box_width);
}

/*
* Initialization function
*
* for both constructors
* 
*/
void City::init(PNG* input, int32_t grid_box_width)
{
    // init seed for srand
    srand(SEED);
    // init basic private variables
    cityWidth = input->width();
    cityLength = input->height();
    input_img = input;
    grid_cell_size = grid_box_width;
    cell_cnt = 0;
    // add outer edge of grid cells around original image
    gridWidth = cityWidth/(grid_cell_size + GRID_SPACE) + 2;
    gridLength = cityLength/(grid_cell_size + GRID_SPACE) + 2;
    // get area of cell and space
    int grid_cell_area = pow(grid_cell_size,2);

    // init grid cells
    for(int i = 0; i < gridLength; i++){
        // create empty row vector
        vector<gridCell*> row(gridWidth);
        for(int j = 0; j < gridWidth; j++){
            gridCell* cell = new gridCell;
            row[j] = cell;

            // the corner position needs to be -squarew,-squarew
            cell->pos_x = j * (grid_cell_size + GRID_SPACE) - (grid_cell_size);
            cell->pos_z = i * (grid_cell_size + GRID_SPACE) - (grid_cell_size);
            cell->pos_y = 0;
            cell->i_index = i;
            cell->j_index = j;
            cell->visited = false;
            // cells default to none type
            cell->type = 0;

            // find if the cell needs to be in city or not
            int count = 0;
            // loop for each pixel within the current grid cell
            for(int k = cell->pos_x; k < cell->pos_x + grid_cell_size; k++){
                for(int l = cell->pos_z; l < cell->pos_z + grid_cell_size; l++){
                    // check if checked pixel is in bounds
                    if(k>=0 && l>=0 && k<cityWidth && l<cityLength){
                        // get the current pixel
                        HSLAPixel & pixel = input->getPixel(k, l);
                        // check whether the luminance value is equal to the white pixel
                        if((int32_t)pixel.l == WHITE_PX_LUM){
                            count++;
                        }
                    }
                }
            }

            // check whether enough pixels are white
            if((double)count / (double)grid_cell_area > PX_PCT){
                cell->inCity = true;
                cell_cnt++;
                // add cell to list
                cityCells.push_back(cell);
            }
            else{
                cell->inCity = false;
            }
        }
        // add row to grid
        grid.push_back(row);
    }

    // set height of city if needed
    if(heightmap_img != NULL){
        setCityHeight();
    }

    // set the type of each cell prior to maze generation
    // uses "brute force edge detection"
    for(int i = 1; i < gridLength - 1; i++){
        for(int j = 1; j < gridWidth - 1; j++){
            gridCell* cell = grid[i][j];
            if(cell->inCity){
                // count 8 surrounding cells 
                int edge_cnt = 0;
                int height_cnt = 0;
                for(int k = -1; k <= 1; k++){
                    for(int l = -1; l <= 1; l++){
                        // count city cells in surrounding cells
                        if(grid[i+k][j+l]->inCity){
                            edge_cnt++;
                        }
                        // count lower height surrounding cells
                        if(grid[i+k][j+l]->pos_y < cell->pos_y){
                            height_cnt++;
                        }
                    }
                }
                // set it as a border if enough of its surrounding cells are in the city
                if((double)(edge_cnt-1) / NUM_ADJ < OUTER_RD_PCT){
                    cell->type = 3;
                }
                // set it as a cliff if enough surrounding cells are different height
                else if((double)(height_cnt) / NUM_ADJ > HEIGHT_PCT){
                    cell->type = 4;
                }
                // otherwise set it as a house
                else{
                    cell->type = 2;
                }
            }
        }
    }

    generateRoads();

    // generate maze in city

    // each list cell will have coords same as image pixel
}

/*
* setCityHeight
*
* sets the height of the city to input heightmap
*
*/
void City::setCityHeight()
{
    double grid_cell_area = (double)pow(grid_cell_size,2);
    // loop through city cells
    for(auto cell: cityCells){
        double lum_sum = 0.0;
        // loop through pixels in cell
        for(int k = cell->pos_x; k < cell->pos_x + grid_cell_size; k++){
            for(int l = cell->pos_z; l < cell->pos_z + grid_cell_size; l++){
                // check if checked pixel is in bounds
                if(k>=0 && l>=0 && k<cityWidth && l<cityLength){
                    // get the current pixel
                    HSLAPixel & pixel = heightmap_img->getPixel(k, l);
                    // sum luminance
                    lum_sum += pixel.l;
                }       
            }
        }
        // calculate average
        double avg = lum_sum / grid_cell_area;
        // scale average and truncate
        cell->pos_y = (int)floor(NUM_BUCKETS*avg);
    }
}

/*
* generateRoads
*
* generates maze of roads inside city where cliffs dont exsist
*
*/
void City::generateRoads()
{
    // stack<gridCell*> frontier;
    // frontier.push(cityCells.front());
    // cityCells.front()->visited = true;

    // while(frontier.size != 0){
    //     gridCell* cur = frontier.top();
    //     frontier.pop();

    //     list<gridCell*>* neighbors = get4Neighbors(cur);
    //     for(auto: cell: *neighbors){
    //         if(!(cell->visited)){
                
    //         }
    //     }
    // }

    // generate a maze on all points that are inCity
    // do BFS for each node
    // use for loops to get 4 adjacent cells

    // go straight with higher probability than left or right
    // mark cells as roads as maze is generated
}

/*
* randTF
*
* generates a true/false random output with given dist
* dist is % chance it will be true
* only does integer percentages
*
*/
bool City::randTF(int32_t dist)
{
    // random num between 0 and 1000
    int x = rand() % 100;
    // true if the rand is <= to the dist
    return(x <= dist);
}

/*
* get4Neighbors
*
* given input cell, gets 4 adjacent neighbors
* output needs to be freed later
*
*/
std::list<City::gridCell*>* City::get4Neighbors(gridCell* cell)
{
    list<gridCell*>* output = new list<gridCell*>;
    // hardcoded adjacent cells
    output->push_back(grid[cell->i_index][cell->j_index+1]);
    output->push_back(grid[cell->i_index][cell->j_index-1]);
    output->push_back(grid[cell->i_index+1][cell->j_index]);
    output->push_back(grid[cell->i_index-1][cell->j_index]);
    return(output);
}

/*
* PrintGrid
*
* prints out the grid of the city to "output.png"
*
*/
void City::printGrid()
{
    HSLAPixel red_px(0.0,1.0,0.5);
    HSLAPixel green_px(120,1.0,0.5);
    HSLAPixel blue_px(240,1.0,0.5);
    HSLAPixel orange_px(39,1.0,0.5);
    HSLAPixel black_px(0,1.0,0.5);

    PNG input_cpy = (*input_img);

    // loop through cells
    for(auto cell: cityCells){
        // find color that cell should be
        HSLAPixel color;
        switch (cell->type){
            case 0:
                color = black_px;
                break;
            case 1:
                color = blue_px;
                break;
            case 2:
                color = red_px;
                break;
            case 3:
                color = orange_px;
                break;
            case 4:
                color = green_px;
                break;
        }
        // loop through each pixel in the cell
        for(int k = cell->pos_x; k < cell->pos_x + grid_cell_size; k++){
            for(int l = cell->pos_z; l < cell->pos_z + grid_cell_size; l++){
                // check if checked pixel is in bounds
                if(k>=0 && l>=0 && k<cityWidth && l<cityLength){
                    // set the pixel to the cell's color
                    HSLAPixel & pixel = input_cpy.getPixel(k, l);
                    pixel = color;
                }
            }
        }
    }
    input_cpy.writeToFile("output.png");
}

/*
* Destructor
*
* frees memory
*
*/
City::~City(){
    for(int i = 0; i < gridLength; i++){
        for(int j = 0; j < gridWidth; j++){
            free(grid[i][j]);
        }
    }
}