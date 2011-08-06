/***
***
***		structs.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

typedef struct{
   char * name;
   int quantity;
}package;

typedef struct{
   char * destination;
   int distance;
}arc;

typedef struct{
   char * name;
   package ** medicines;
   arc ** adjacents;
   unsigned ID;
}city;

typedef struct{
   package ** medicines;
   char * startCity;
   unsigned companyID;
}plane;

typedef struct{
   plane ** companyPlanes;
   int planesCount;
   unsigned ID;
}company;

typedef struct{
   company ** mapCompanies;
   city ** mapCities;
   int ** graph;
   int citiesCount;
   int companiesCount;
}map;

typedef struct{
	FILE * file;
	int line;
	int valid;
}mapData;
