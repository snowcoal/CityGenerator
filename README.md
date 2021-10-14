# CityGenerator
Generates cities for Minecraft

by snowcoal

Overview:


This is a program that generates complex cities for minecraft. The program essentially first generates a city layout, then places houses into it, and finally generates a list of minecraft and worldedit
commands that can be run in order to completely generate the city. It requires that the user builds a set of houses that are to be used to make up the city.


Detailed Description:


The program is made up of two main classes, City, and HouseSet. These are defined in City.h and HouseSet.h respectively. The City class takes care of generating the entire city, as well as placing the houses
into it, and also outputting the city as a list of commands. The HouseSet class takes care of loading the house list that the user wishes to use, as well as storing this list for the city class to use.

To generate a city, the user must first instantiate a HouseSet object. To instantiate a HouseSet object, an input .csv file MUST be provided that has data about each house that is to be used. See video (upcoming) for more details about
how to format this file. The program DOES NOT currently check this file for accuracy, so any issues with it WILL cause issues in the final output. Additionally,
to build a city, there must be certain types of houses provided. The program somewhat checks that all required houses exist, and it will output an error if it attempts
to use a house that doesnt exist. Again, see (upcoming) video for more details on this.

After instantiating a HouseSet object, the user can then instantiate a City object (technically you can instantiate a city without a HouseSet, but you will not be able to output the city to minecraft). The City class has
two constructors. One generates a flat city, and the other generates a city that follows a heightmap (needs some work).

After instantiating a City object, its methods MUST be called in the following order (assuming the user wishes to use them). Correct usage is shown in the given examples.
The code DOES check for correct order and will display errors if the order is incorrect.

1. addRandomRoads()     (optional)
2. placeHouses()        (required)
3. distortCity()        (optional)
4. outputCity()         (required, and requires instantiation of a HouseSet object)

printGrid() can be called at any point after the initial construction, and can be used to see the the city grid and house placement.
If it is called before placeHouses(), it will display no houses, and only the grid, and if it is called after, it will display the houses overlayed on the grid.


Notes:


Yes I know that I probably wrote too many comments. I have borderline OCD when it comes to comments so thats my problem.

Yes I know that the grid is overly complicated. And yes I know each grid cell stores its position within the image even though it doesnt need to, as I can just directly calculate each gridcell position.
The reason why I did this was becuase originally I wanted the grid cells to be able to be moved, if the city is distorted. Now Ive realized that its probably better to distort the houses instead if I wish
to add this functionality, but I'm not entirely sure. So for now, they will continue to store their current position in the event that I wish to be able to move them around. Also, all of the calculations
related to gridcell positions are based off of the stored position, so removing this functionality would be annyoing.

Also, Im aware that some parts of the program (such as edge detection and heightmap calculations) are brutally inefficient and I should be using a better method. I probably should be, but at the end of the
day this program runs in under a second, and the resulting city takes over an hour to build inside minecraft by auto-running the commands. So the program is not the limiting factor here. Also, I did try to
optimize the program whenever possible.



