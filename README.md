# CityGenerator
Generates cities for Minecraft

Things to note:

There is no copy constructor or operator overloads for the city class

After constructing a City object, its methods MUST be called in the following order (assuming the user wishes to use them). Correct usage is shown in the given examples.
The code DOES check for correct order and will display errors if the order is incorrect.

    1. addRandomRoads()     (optional)
    2. placeHouses()        (required)
    3. distortCity()        (optional)
    4. outputCity()         (required)

    printGrid() can be called at any point after the initial construction, and can be used to see the the city grid and house placement.
    If it is called before placeHouses(), it will display no houses, and if it is called after, it will display the houses overlayed on the grid.



