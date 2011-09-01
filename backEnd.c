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

#define SIZECOL 6
#define SIZEROW 6

int matrix[SIZECOL][SIZEROW] = { { 0, 5, -1, 15, -1, -1 },
		{ 5, 0, 8, -1, 7, -1 }, { -1, 8, 0, -1, -1, 20 },
		{ 15, -1, -1, 0, 2, -1 }, { -1, 7, -1, 2, 0, -1 }, { -1, -1, 20, -1, -1,
				0 } };

/***		Functions		***/
static int getCity(FILE * mapFile, city * newCity);
static int getInt(FILE * mapFile, int * out);
static int getString(FILE * mapFile, char ** out);
void dijkstra();

int openFile(FILE ** file, char * fName) {
	char * string;

	if ((string = malloc(strlen(fName) + 3)) == NULL)
	{
		return 1;
	}
	strcpy(string, "./"); /* Se busca el archivo en la ubicacion actual */
	strcat(string, fName);
	*file = fopen(string, "r");
	free(string);
	if (*file)
		return 0;

	return 1;
}

int allocMapSt(map ** mapSt, int argc) {
	if ((*mapSt = malloc(sizeof(map))) == NULL)
	{
		return 1;
	}
	return 0;
}

int createCities(FILE * file, map * mapSt) {
	int i;
	char * aux;
	getInt(file, &(mapSt->citiesCount)); /* gets the quantity of cities */

	if (getString(file, &aux) != BLANKLINE)
		return 1;

	if ((mapSt->cities = malloc(mapSt->citiesCount * sizeof(city*))) == NULL)
		return 1;

	for (i = 0; i < mapSt->citiesCount; i++) /* initialices the cities */
	{
		if ((mapSt->cities[i] = malloc(sizeof(city))) == NULL
			)
			return 1;

		if (getCity(file, mapSt->cities[i]))
			return 1;

		mapSt->cities[i]->ID = i;
	}

	if (initializeGraph(file, mapSt))
		return 1;

	return 0;
}

static int getCity(FILE * file, city * newCity) {
	int packages = 0; /*medicines*/
	char * aux;

	if (getString(file, &(newCity->name))) {
		return 1;
	}

	while (getString(file, &aux) != BLANKLINE) /*revisar*/
	{ /*gets the medicine name*/
		if ((newCity->medicines = realloc(newCity->medicines,
				sizeof(medicine*) * ++packages)) == NULL) {
			return 1;
		}
		if ((newCity->medicines[packages - 1] = malloc(sizeof(medicine)))
				== NULL) {
			return 1;
		}
		newCity->medicines[packages - 1]->name = aux;
		if (getInt(file, &(newCity->medicines[packages - 1]->quantity))) {/*gets the medicine quantity*/
			return 1;
		}
	}
	newCity->medCount = packages;

	return 0;
}

int initializeGraph(FILE * file, map * mapSt) {
	int i, ID1, ID2, ret, dist;
	char * aux;

	if ((mapSt->graph = calloc(mapSt->citiesCount, sizeof(int *))) == NULL
		)
		return 1;

	for (i = 0; i < mapSt->citiesCount; i++)
		if ((mapSt->graph[i] = calloc(mapSt->citiesCount, sizeof(int))) == NULL
			)
			return 1;

	while ((ret = getString(file, &aux)) != ENDEDFILE) /*revisar*/
	{
		if (ret != BLANKLINE)
		{
			ID1 = getCityID(aux, mapSt);
			if (getString(file, &aux))
				return 1;
			ID2 = getCityID(aux, mapSt);
			if (getInt(file, &dist)) /* gets the distance to a city */
				return 1;
			if (ID1 == -1 || ID2 == -1)
				return 1;
			mapSt->graph[ID1][ID2] = dist;
			mapSt->graph[ID2][ID1] = dist;
		}
	}

	return 0;
}

int createCompany(FILE * file, company ** newCompany) {
	int qtty, i;
	char * aux;

	if ((*newCompany = malloc(sizeof(company))) == NULL) {
		printf("Could not allocate company memory\n");
		return 1;
	}

	if (getInt(file, &qtty)) {
		printf("Could not get planes quantity\n");
		return 1;
	}

	(*newCompany)->planesCount = qtty;

	if (((*newCompany)->companyPlanes = malloc(sizeof(plane*) * qtty))
			== NULL) {
		printf("Could not allocate company planes memory\n");
		return 1;
	}

	getString(file, &aux);
	/*if( getString(mapFile, &aux) != BLANKLINE )
	 {
	 printf("File error. Not a blank line before first company plane\n");
	 return 1;
	 }*/

	for (i = 0; i < qtty; i++) {
		if (createPlane(file, &((*newCompany)->companyPlanes[i])))
			return 1;
		(*newCompany)->companyPlanes[i]->planeID = i;
	}

	return 0;
}

int createPlane(FILE * file, plane ** newPlane) {
	char * aux;
	int packages = 0, ret;

	if (getString(file, &aux))
		return 1;
	if ((*newPlane = malloc(sizeof(plane))) == NULL)
		return 1;

	(*newPlane)->startCity = aux;

	while ((ret = getString(file, &aux)) != BLANKLINE && ret != ENDEDFILE) /*revisar*/
	{
		if (((*newPlane)->medicines = realloc((*newPlane)->medicines,
				sizeof(medicine*) * ++packages)) == NULL)
			return 1;
		if (((*newPlane)->medicines[packages - 1] = malloc(sizeof(medicine)))
				== NULL)
			return 1;
		(*newPlane)->medicines[packages - 1]->name = aux;
		if (getInt(file, &((*newPlane)->medicines[packages - 1]->quantity)))
			return 1;
	}
	(*newPlane)->medCount = packages;

	return 0;
}

int getCityID(char * cityName, map * mapSt) {
	int i;
	for (i = 0; i < mapSt->citiesCount; i++) {
		if (!strcmp(mapSt->cities[i]->name, cityName)) {
			return mapSt->cities[i]->ID;
		}
	}
	return -1;
}

static int getInt(FILE * file, int * out) {
	int cant, start = 0, c;
	char * aux = NULL;

	while (!start && (c = fgetc(file)) != EOF) {
		if (isdigit(c)) {
			start = 1;
			cant = 0;
			if ((aux = malloc(sizeof(char) * BLOCK)) == NULL)
			{
				return 1;
			}
			do {
				aux[cant++] = c;
				c = fgetc(file);
			} while (isdigit(c));
			aux[cant] = '\0';
		}

	}
	if (aux) {
		*out = atoi(aux);
		free(aux);
		return 0;
	}
	return 1;
}

static int getString(FILE * file, char ** out) {

	int cant, start = 0, c, cap = BLOCK, newLine = 0;
	char * aux = NULL;

	while (!start && (c = fgetc(file)) != EOF) {

		if ((c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == CR)
				&& !start) {
			if (newLine)
				return BLANKLINE;
			if (c == '\n') {
				newLine = 1;
			}
		}
		if (isalnum(c)) {
			start = 1;
			cant = 0;
			if ((aux = malloc(sizeof(char) * cap)) == NULL)
			{
				return 1;
			}

			do {
				if (cap == cant) {
					cap *= INCREASERATIO;
					if ((aux = realloc(aux, sizeof(char) * cap)) == NULL) {
						free(aux);
						return 1;
					}
				}
				aux[cant++] = c;
				c = fgetc(file);
			} while (isalnum(c));
			aux[cant] = '\0';
		}
	}
	if (aux) {
		*out = aux;
		return 0;
	}
	if (c == EOF)
		return ENDEDFILE;
	return 1;
}

void dijkstra() {
	int row, col, k, min, sum_aux,i,j, nuevacol, minFila,viejafila;

	/* Imprimo antes la matriz*/
	for( i = 0; i < SIZEROW; i++) {
		for ( j = 0; j < SIZECOL; j++) {
			printf("%d ",matrix[i][j]);
		}
		printf("\n");
	}

	/*Empieza el MEGA-Algorithm (?)*/
	for(row = 0; row < SIZEROW; row++) {
		for(col=0; col < SIZECOL; col++) {
			sum_aux = 0;
			if( matrix[row][col] == -1) {
				nuevacol = col;
				min = -1;
				viejafila = -1;
				while( matrix[row][nuevacol] == -1) {
					min = -1;
					for ( k = 0; k < SIZEROW; k++ ) {
						if ( min == -1 && matrix[k][nuevacol] > 0 && k != viejafila)
						{
							min = matrix[k][nuevacol];
							minFila = k;
						} else if( matrix[k][nuevacol] > 0 && matrix[k][nuevacol] < min && k != viejafila)
						{
							min = matrix[k][nuevacol];
							minFila = k;
						}
						/*if ( ( matrix[k][col] > 0 ) && (matrix[row][k] > 0) ){
						 sum_aux = 0;
						 sum_aux = matrix[row][k] + matrix [k][col];
						 if ( sum_aux < min || (matrix[row][col] == -1) ){
						 matrix[row][col] = sum_aux;
						 matrix[col][row] = sum_aux;
						 min = sum_aux;
						 }
						 }*/
					}
					sum_aux += min;
					viejafila= nuevacol;
					nuevacol = minFila;

				}
				sum_aux += matrix[row][nuevacol];
				matrix[row][col] = sum_aux;
				matrix[col][row] = sum_aux;
				/*if ( sum_aux < matrix[row][col] || (matrix[row][col] == -1) ){
				 matrix[row][col] = sum_aux;
				 matrix[col][row] = sum_aux;
				 min = sum_aux;
				 }*/

			}
		}
	}
	printf("\nMATRIZ TERMINADA\n");
	/*Termino imprimendo el resultado*/
	for( i = 0; i < SIZEROW; i++) {
		for ( j = 0; j < SIZECOL; j++) {
			printf("%d ",matrix[i][j]);
		}
		printf("\n");
	}
}
