# CityGenerator
Generates cities for Minecraft

Things to note:

There is no copy constructor or operator overloads for the City class or HouseSet class, so do not try to use these operators.

After constructing a City object, its methods MUST be called in the following order (assuming the user wishes to use them). Correct usage is shown in the given examples.
The code DOES check for correct order and will display errors if the order is incorrect.

1. addRandomRoads()     (optional)
2. placeHouses()        (required)
3. distortCity()        (optional)
4. outputCity()         (required, and requires instantiation of a HouseSet object)

printGrid() can be called at any point after the initial construction, and can be used to see the the city grid and house placement.
If it is called before placeHouses(), it will display no houses, and only the grid, and if it is called after, it will display the houses overlayed on the grid.

To instantiate a HouseSet object, an input .csv file MUST be provided that has data about each house that is to be used. See video (upcoming) for more details about
how to format this file. The program DOES NOT currently check this file for accuracy, so any issues with it WILL cause issues in the final output. Additionally,
to build a city, there must be certain types of houses provided. The program somewhat checks that all required houses exist, and it will output an error if it attempts
to use a house that doesnt exist. Again, see (upcoming) video for more details on this.





