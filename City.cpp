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
    gridWidth = cityWidth/(grid_cell_size + GRID_SPACE) + 4;
    gridLength = cityLength/(grid_cell_size + GRID_SPACE) + 4;
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
    for(int i = 2; i < gridLength - 2; i++){
        for(int j = 2; j < gridWidth - 2; j++){
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
                    cell->visited = true;
                }
                // set it as a cliff if enough surrounding cells are different height
                else if((double)(height_cnt) / NUM_ADJ > HEIGHT_PCT){
                    cell->type = 4;
                }
                // set it as a house if it has an odd position
                else if(!(i%2 == 0 && j%2 == 0)){
                    cell->type = 2;
                }
                // finally set it as a road
                else{
                    cell->type = 1;
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
* returns 0 on success, -1 on failure
*
*/
int32_t City::generateRoads()
{
    // choose initial cell
    gridCell* initial = NULL;
    for(auto cell: cityCells){
        // find first road cell and break
        if(cell->type == 1){
            initial = cell;
            break;
        }
    }
    // if no initial cell is found, return -1
    if(initial == NULL){
        return -1;
    }

    // mark inital as visited and push to stack
    stack<gridCell*> frontier;
    frontier.push(initial);
    initial->visited = true;

    while(frontier.size() != 0){
        // current cell is top of stack
        gridCell* cur = frontier.top();
        frontier.pop();

        // if current has a neighbor
        gridCell* n = pickRandNeighbor(cur, 80);
        if(n != NULL){
            // push current to stack
            frontier.push(cur);

            // set wall between them to a road
            int32_t i_diff = (n->i_index - cur->i_index) >> 1;
            int32_t j_diff = (n->j_index - cur->j_index) >> 1;
            grid[cur->i_index + i_diff][cur->j_index + j_diff]->type = 1;
            //grid[cur->i_index + i_diff][cur->j_index + j_diff]->visited = true;

            // mark neighbor as visited and push it to stack
            n->visited = true;
            frontier.push(n);
        }
    }
    // return success
    return 0;

    // Choose the initial cell, mark it as visited and push it to the stack
    //     While the stack is not empty
    //         Pop a cell from the stack and make it a current cell
    //         If the current cell has any neighbours which have not been visited
    //             Push the current cell to the stack
    //             Choose one of the unvisited neighbours
    //             Remove the wall between the current cell and the chosen cell
    //             Mark the chosen cell as visited and push it to the stack

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
* pickRandNeighbor
*
* given input cell, picks random neighboring adjacent cell
* that has not been visited. Used by maze generator.
* neighboring cell HAS to be 2 spaces away
*
* input lrbias is left/right bias as opposed to up/down
* 
* returns null if cell has no unvisited neighbors
*/
City::gridCell* City::pickRandNeighbor(gridCell* cell, int32_t lrbias)
{
    vector<gridCell*> cells(4);
    uint32_t order;
    gridCell* output = NULL;
    // right/left
    cells[0] = grid[cell->i_index][cell->j_index+2];
    cells[1] = grid[cell->i_index][cell->j_index-2];
    // down/up
    cells[2] = grid[cell->i_index+2][cell->j_index];
    cells[3] = grid[cell->i_index-2][cell->j_index];

    // if true then left/right comes first
    bool lr = randTF(lrbias);
    // if true then plus comes first
    bool plus = randTF(50);

    /*  lr  p | 1st | 2nd | 3rd | 4th | bin encode  | hex
    * --------|-----|-----|-----|-----|-------------|------
    *    0  0 |  3  |  2  |  1  |  0  | 11 10 01 00 | 0xe4
    *    0  1 |  2  |  3  |  0  |  1  | 10 11 00 01 | 0xb1
    *    1  0 |  1  |  0  |  3  |  2  | 01 00 11 10 | 0x4e
    *    1  1 |  0  |  1  |  2  |  3  | 00 01 10 11 | 0x1b
    */

    // set the order according to the above table
    if(!lr && !plus) order = 0xe4;
    else if(!lr && plus) order = 0xb1;
    else if(lr && !plus) order = 0x4e;
    else order = 0x1b;
    //std::cout<<order<<std::endl;
    for(int n = 3; n >= 0; n--){
        // do only if cell is a road and is not visited
        if(!(cells[n]->visited) && cells[n]->type == 1){
            // update output
            output = cells[(order >> (3-n)*2) & BITMASK];
        }
    }

    // sanity check
    if(output != NULL){
        assert(output->type == 1 && !(output->visited));
    }

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