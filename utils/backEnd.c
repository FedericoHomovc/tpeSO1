/***
***
***		backEnd.c
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/


/***		System includes		***/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/***		Project Includes		***/
#include "../include/structs.h"
#include "../include/backEnd.h"

/***		Module Defines		***/
#define BLOCK 32
#define INCREASERATIO 1.3
#define BLANKLINE 2
#define ENDEDFILE -1
#define CR 13

/***		Functions		***/

/*
* getCity
*
* This function creates a new City struct and fills it with the values read
* from the file. 
*
* @mapFile: pointer to the file from where to get the data.
* @newCity: pointer to the struct where the data will be put.
*/
static int getCity( FILE * mapFile, city * newCity);

/*
* getInt
*
* Reads an integer from the file given and returns it in the pointer given as
* parameter.
*
* @mapFile: pointer to the file from where to get the data.
* @out: pointer to the integer where the data will be put.
*/
static int getInt(FILE * mapFile, int * out);

/*
* getString
*
* Reads a string from the file given and returns it in the pointer given as
* parameter. Returns ENDEDFILE if there is nothing more to read and BLANKLINE
* if the current line is empty.
*
* @mapFile: pointer to the file from where to get the data.
* @out: pointer to the string where the data will be put.
*/
static int getString(FILE * mapFile, char ** out);

/*
* initializeGraph
*
* Reads the data from the file given and creates a square matrix with all the 
* distances between the cities.
*
* @file: pointer to the file from where to get the data.
* @mapSt: pointer to the struct where the data will be put.
*/
static int initializeGraph(FILE * file, map * mapSt);

/*
* createPlane
*
* Creates a new plane with the data read from the file given.
*
* @file: pointer to the file from where to get the data.
* @newPlane: pointer to the struct where the data will be put.
*/
static int createPlane(FILE * file, plane ** newPlane);

/*
* floyd
*
* Applies the Floyd-Warshall algorithm to the matrix given.
*
* @matrix: pointer to the matrix to applie the algorithm.
* @size: number of rows and columns of the matrix.
*/
static void floyd(int *** matrix, int size);

int
openFile(FILE ** file, char * fName)
{
	char * string = NULL;
	
	if( (string = malloc( strlen(fName) + 3)) == NULL)
		return 1;
	strcpy(string, "./");
	strcat(string, fName);
	*file = fopen(string,"r");
	free(string);
	if(*file)
		return 0;

	return 1;
}

int
createCities(FILE * file, map * mapSt)
{
	int i;
	char * aux = NULL;
	getInt( file,  &(mapSt->citiesCount) ); /* gets the quantity of cities */


	if( getString(file, &aux) != BLANKLINE )
		return 1;

	if( (mapSt->cities = malloc(mapSt->citiesCount * sizeof(city*))) == NULL )
		return 1;
	
	for( i = 0; i < mapSt->citiesCount; i++ ) /* initialices the cities */
	{
		if( (mapSt->cities[i] = malloc(sizeof(city))) == NULL)
			return 1;

		if( getCity(file, mapSt->cities[i]) )
			return 1;

		mapSt->cities[i]->ID = i;
	}

	if( initializeGraph(file, mapSt) )
		return 1;
	floyd(&mapSt->graph, mapSt->citiesCount);

	return 0;
}

static int
getCity( FILE * file, city * newCity)
{
	int packages = 0; /*medicines*/
	char * aux = NULL;
	
	if( getString(file, &(newCity->name)) ) /*gets the name of the city*/
		return 1;

	if( (newCity->medicines = malloc(1)) == NULL)	/*to initially alloc*/
		return 1;

	while( getString( file, &aux ) != BLANKLINE )
	{
 		if ( (newCity->medicines = realloc(newCity->medicines, sizeof(medicine*) * ++packages)) == NULL )
			return 1;

		if( (newCity->medicines[packages-1] = malloc(sizeof(medicine))) == NULL )
			return 1;

		newCity->medicines[packages-1]->name = aux;
		if( getInt(file, &(newCity->medicines[packages-1]->quantity) ) ) /*gets the medicine quantity*/
			return 1;	
	}
	newCity->medCount = packages;

	return 0;
}

static int
initializeGraph(FILE * file, map * mapSt)
{
	int i, ID1, ID2, ret, dist;
	char * aux = NULL;

	if( (mapSt->graph = calloc(mapSt->citiesCount, sizeof(int *))) == NULL)
		return 1;
	
	for( i = 0; i<mapSt->citiesCount; i++ )
		if( (mapSt->graph[i] = calloc(mapSt->citiesCount, sizeof(int))) == NULL)
			return 1;

	while( (ret = getString( file, &aux )) != ENDEDFILE)
	{	
		if(ret != BLANKLINE)
		{
			ID1 = getCityID(aux, mapSt);
			free(aux);
			if( getString( file, &aux ) )
				return 1;
			ID2 = getCityID(aux, mapSt);
			free(aux);
			if( getInt( file, &dist ) ) /*gets the distance to a city*/
				return 1;
			if( ID1 == -1 || ID2 == -1 )
				return 1;
			mapSt->graph[ID1][ID2] = dist;
			mapSt->graph[ID2][ID1] = dist;
		}
	}	

	return 0;
}

int
createCompany(FILE * file, company ** newCompany)
{
	int qtty, i;
	char * aux = NULL;

	if( (*newCompany = malloc(sizeof(company))) == NULL )
	{
		fprintf(stderr, "Could not allocate company memory\n");
		return 1;
	}

	if(getInt( file, &qtty ))
	{
		fprintf(stderr, "Could not get planes quantity\n");
		return 1;
	}

	(*newCompany)->planesCount = qtty;

	if( ((*newCompany)->companyPlanes = malloc(sizeof(plane*) * qtty)) == NULL )
	{
		fprintf(stderr, "Could not allocate company planes memory\n");
		return 1;
	}
	
	getString(file, &aux);

	for( i = 0; i<qtty; i++)
	{
		if(createPlane(file, &((*newCompany)->companyPlanes[i]) ))
			return 1;
		(*newCompany)->companyPlanes[i]->planeID = i;
	}

	return 0;
}

static int
createPlane(FILE * file, plane ** newPlane)
{
	char * aux = NULL;
	int packages = 0, ret;
	
	if( getString(file, &aux) )
		return 1;
	if( (*newPlane = malloc(sizeof(plane))) == NULL )
		return 1;

	(*newPlane)->startCity = aux;

	if( ((*newPlane)->medicines = malloc(1)) == NULL) /*to initially alloc*/
		return 1;

	while( (ret = getString( file, &aux )) != BLANKLINE && ret != ENDEDFILE)
	{
		if ( ((*newPlane)->medicines = realloc((*newPlane)->medicines, sizeof(medicine*) * ++packages)) == NULL )
			return 1;

		if( ((*newPlane)->medicines[packages-1] = malloc(sizeof(medicine))) == NULL )
			return 1;

		(*newPlane)->medicines[packages-1]->name = aux;
		if( getInt(file, &((*newPlane)->medicines[packages-1]->quantity) ) )
			return 1;	
	}
	(*newPlane)->medCount = packages;
	(*newPlane)->distance = 0; /*initial distance to start city is set to 0*/

	return 0;
}

int
getCityID( char * cityName, map * mapSt)
{
	int i;
	for(i = 0; i<mapSt->citiesCount; i++)
	{
		if(! strcmp(mapSt->cities[i]->name, cityName))
			return mapSt->cities[i]->ID;
	}
	return -1;
}


static int
getInt(FILE * file, int * out)
{
	int cant, start = 0, c;
	char * aux = NULL;

	while( !start && (c=fgetc(file)) != EOF)
	{
		if(isdigit(c))
		{
			start = 1;
			cant = 0;
			if( (aux = malloc(sizeof(char) * BLOCK)) == NULL)
				return 1;
			do
			{
				aux[cant++] = c;
				c=fgetc(file);
			}while(isdigit(c));
			aux[cant] = '\0';
		}
	}

	if(aux)
	{
		*out=atoi(aux);
		free(aux);
		return 0;
	}
	return 1;
}

static int
getString(FILE * file, char ** out)
{

	int cant, start = 0, c, cap = BLOCK, newLine = 0;
	char * aux = NULL;

	while(!start && (c=fgetc(file))!=EOF )
	{
		if(( c==' ' || c=='\t' || c=='\r' || c=='\n' || c==CR) && !start)
		{
			if(newLine)
				return BLANKLINE;
			if(c == '\n')
				newLine = 1;
		}
		if(isalnum(c))
		{
			start = 1;
			cant = 0;
			if( (aux = malloc(sizeof(char) * cap)) == NULL)
				return 1;

			do
			{
				if(cap == cant)
				{
					cap *= INCREASERATIO;
					if ( ( aux = realloc( aux, sizeof(char)*cap ) ) == NULL )
					{
						free(aux);
						return 1;
					}
				}
				aux[cant++]=c;
				c=fgetc(file);
			}while(isalnum(c));
			aux[cant]='\0';
		}		
	}
	if(aux)
	{
		*out=aux;
		return 0;
	}
	if( c == EOF )
	{
		free(aux);
		return ENDEDFILE;
	}
	return 1;
}

static void
floyd(int *** map, int size){

	int i, j, k;
	int ** dist = NULL;

	dist = *map;

	for (k = 0; k < size; ++k) {
		for (i = 0; i < size; ++i)
			for (j = 0; j < size; ++j)
				if ((dist[i][k] * dist[k][j] != 0) && (i != j))
					if ((dist[i][k] + dist[k][j] < dist[i][j]) || (dist[i][j] == 0))
						dist[i][j] = dist[i][k] + dist[k][j];
	}	

	*map = dist;
}

void itoa(int n, char *string)
{
	int i = 0, sign;

	if ((sign = n) < 0)
		n = -n;
	do {
		string[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);

	if (sign < 0)
		string[i++] = '-';
	string[i] = '\0';
	reverse(string);

	return;
}

void reverse(char *string)
{
	int i, j;
	char c;

	for (i = 0, j = strlen(string)-1; i<j; i++, j--) {
		c = string[i];
		string[i] = string[j];
		string[j] = c;
	}
}
