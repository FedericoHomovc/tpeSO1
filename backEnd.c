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
#include "./include/structs.h"
#include "./include/backEnd.h"

/***		Module Defines		***/
#define BLOCK 32
#define INCREASERATIO 1.3
#define BLANKLINE 2
#define ENDEDFILE -1
#define CR 13


static int getCity( mapData * mapFile, city * newCity);
static int getInt(mapData * mapFile, int * out);
static int getString(mapData * mapFile, char ** out);


int
allocMapData(mapData ** mapFile)
{
	if( (*mapFile = malloc(sizeof(mapData))) == NULL)
	{
		return 1;
	}
	(*mapFile)->valid = 0; 	/*Inicializacion. Vale siempre 0 si no hay error*/
	return 0;
}

int
openFile(mapData * map, char * fName)
{
	char * string;
	
	if( (string=malloc( strlen(fName) + 3)) == NULL)
	{
		return 1;
	}
	strcpy(string, "./"); /* Se busca el archivo en la ubicacion actual */
	strcat(string, fName);
	map->file = fopen(string,"r");
	free(string);
	if(map->file)
	{
		map->line=1; /* Inicializacion */
		return 0;
	}
	return 1;
}

int
allocMapSt(map ** mapSt, int argc)
{
	if( (*mapSt=malloc(sizeof(map))) == NULL)
	{
		return 1;
	}
	if( ( ((*mapSt)->companies) = malloc((argc - 2) * sizeof(company*)) ) == NULL)
	{
		return 1;
	}
	return 0;
}

int
createCities(mapData * mapFile, map * mapSt)
{
	int i;
	char * aux;
	getInt( mapFile,  &(mapSt->citiesCount) ); /* gets the quantity of cities */


	if( getString(mapFile, &aux) != BLANKLINE )
		return 1;

	if( (mapSt->cities = malloc(mapSt->citiesCount * sizeof(city*))) == NULL )
		return 1;
	
	for( i = 0; i < mapSt->citiesCount; i++ ) /* initialices the cities */
	{
		if( (mapSt->cities[i] = malloc(sizeof(city))) == NULL)
			return 1;

		if( getCity(mapFile, mapSt->cities[i]) )
			return 1;

		mapSt->cities[i]->ID = i;
	}

	if( initializeGraph(mapFile, mapSt) )
		return 1;

	return 0;
}

static int
getCity( mapData * mapFile, city * newCity)
{
	int packages = 0; /*medicines*/
	char * aux;
	
	if( getString(mapFile, &(newCity->name)) )
	{
		return 1;
	}

	while( getString( mapFile, &aux ) != BLANKLINE ) /*revisar*/
	{ /*gets the medicine name*/
 		if ( (newCity->medicines = realloc(newCity->medicines, sizeof(medicine*) * ++packages)) == NULL )
		{
			return 1;
		}
		if( (newCity->medicines[packages-1] = malloc(sizeof(medicine))) == NULL )
		{
			return 1;
		}
		newCity->medicines[packages-1]->name = aux;
		if( getInt(mapFile, &(newCity->medicines[packages-1]->quantity) ) )
		{/*gets the medicine quantity*/
			return 1;	
		}
	}
	newCity->medCount = packages;

	return 0;
}

int
initializeGraph(mapData * mapFile, map * mapSt)
{
	int i, ID1, ID2, ret, dist;
	char * aux;

	if( (mapSt->graph = calloc(mapSt->citiesCount, sizeof(int *))) == NULL)
		return 1;
	
	for( i = 0; i<mapSt->citiesCount; i++ )
		if( (mapSt->graph[i] = calloc(mapSt->citiesCount, sizeof(int))) == NULL)
			return 1;

	while( (ret = getString( mapFile, &aux )) != ENDEDFILE) /*revisar*/
	{	
		if(ret != BLANKLINE)
		{
			ID1 = getCityID(aux, mapSt);
			if( getString( mapFile, &aux ) )
				return 1;
			ID2 = getCityID(aux, mapSt);
			if( getInt( mapFile, &dist ) ) /* gets the distance to a city */
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
createCompany(mapData * mapFile, company ** newCompany)
{
	int qtty, i;
	char * aux;

	if( (*newCompany = malloc(sizeof(company))) == NULL )
		return 1;

	if(getInt( mapFile, &qtty ))
		return 1;

	(*newCompany)->planesCount = qtty;

	if( ((*newCompany)->companyPlanes = malloc(sizeof(plane*) * qtty)) == NULL )
		return 1;
	
	if( getString(mapFile, &aux) != BLANKLINE )
		return 1;

	for( i = 0; i<qtty; i++)
	{
		if(createPlane(mapFile, &((*newCompany)->companyPlanes[i]) ))
			return 1;
	}

	return 0;
}

int
createPlane(mapData * mapFile, plane ** newPlane)
{
	char * aux;
	int packages = 0, ret;
	
	if( getString(mapFile, &aux) )
		return 1;
	if( (*newPlane = malloc(sizeof(plane))) == NULL )
		return 1;

	(*newPlane)->startCity = aux;

	while( (ret = getString( mapFile, &aux )) != BLANKLINE && ret != ENDEDFILE) /*revisar*/
	{
		if ( ((*newPlane)->medicines = realloc((*newPlane)->medicines, sizeof(medicine*) * ++packages)) == NULL )
			return 1;
		if( ((*newPlane)->medicines[packages-1] = malloc(sizeof(medicine))) == NULL )
			return 1;
		(*newPlane)->medicines[packages-1]->name = aux;
		if( getInt(mapFile, &((*newPlane)->medicines[packages-1]->quantity) ) )
			return 1;	
	}
	(*newPlane)->medCount = packages;

	return 0;
}

int
getCityID( char * cityName, map * mapSt)
{
	int i;
	for(i = 0; i<mapSt->citiesCount; i++)
	{
		if(! strcmp(mapSt->cities[i]->name, cityName))
		{
			return mapSt->cities[i]->ID;
		}
	}
	return -1;
}


static int
getInt(mapData * mapFile, int * out)
{
	int cant, start=0, c;
	char * aux=NULL;

	while( !start && (c=fgetc(mapFile->file)) != EOF)
	{
		if(isdigit(c))
		{
			start = 1;
			cant = 0;
			if( (aux = malloc(sizeof(char) * BLOCK)) == NULL)
			{
				return 1;
			}
			do
			{
				aux[cant++] = c;
				c=fgetc(mapFile->file);
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
getString(mapData * mapFile, char ** out)
{

	int cant, start=0, c, cap = BLOCK, newLine = 0;
	char * aux = NULL;

	while(!start && (c=fgetc(mapFile->file))!=EOF )
	{

		if(( c==' ' || c=='\t' || c=='\r' || c=='\n' || c==CR) && !start)
		{
			if(newLine)
				return BLANKLINE;
			if(c == '\n')
			{
				newLine = 1;
			}
		}
		if(isalnum(c))
		{
			start=1;
			cant=0;
			if(	(aux=malloc(sizeof(char) * cap)) == NULL)
			{
				return 1;
			}
			
			do
			{
				if(cap==cant)
				{
					cap *= INCREASERATIO;
					if ( ( aux = realloc( aux, sizeof(char)*cap ) ) == NULL )
					{
						free(aux);
						return 1;
					}
				}
				aux[cant++]=c;
				c=fgetc(mapFile->file);
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
		return ENDEDFILE;
	return 1;
}
