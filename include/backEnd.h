/***
***
***		backEnd.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/


/***		Functions		***/

/*
* openFile
*
* Opens a file with the specified route in fName. Returns 1 if the
* file doesn't exist or if it can't be opened.
*
* @fName: pointer to the name of the file to be opened.
* @file: pointer where the opened file will be returned.
*/
int openFile(FILE ** file, char * fName);

/*
* createCities
*
* Creates all the cities of the corresponding map with their attributes.
*
* @mapFile: pointer to the file from where to get the data.
* @mapSt: map struct where the cities will be created. 
*/
int createCities(FILE * mapFile, map * mapSt);

/*
* createCompany
*
* Creates a company with all its planes.
*
* @mapFile: pointer to the file from where to get the data.
* @newCompany: pointer to the struct where the data will be put.
*/
int createCompany(FILE * mapFile, company ** newCompany);

/*
* getCityID
*
* Given a city name, it returns the corresponding ID or -1 if
* such city doesn't exist.
*
* @cityName: name of the city to search.
* @mapSt: map where the city is taken from.
*/
int getCityID( char * cityName, map * mapSt);

/*
* itoa
*
* Turns an integer into a string and returns it in the parameter given.
*
* @n: integer to be turned into a string.
* @string: string where the result is returned.
*/
void itoa(int n, char * string);

/*
* reverse
*
* Reverses a string given as a parameter.
*
* @string: string to be reversed.
*/
void reverse(char * string);
