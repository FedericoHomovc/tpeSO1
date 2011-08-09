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
}medicine;

typedef struct{
   char * name;
   medicine ** medicines;
   unsigned ID;
}city;

typedef struct{
   medicine ** medicines;
   char * startCity;
   unsigned companyID;
}plane;

typedef struct{
   plane ** companyPlanes;
   int planesCount;
   unsigned ID;
}company;

typedef struct{
   company ** companies;
   city ** cities;
   int ** graph;
   int citiesCount;
   int companiesCount;
}map;

typedef struct{
	FILE * file;
	int line;
	int valid;
}mapData;
