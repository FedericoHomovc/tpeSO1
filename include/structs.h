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
   unsigned medCount;
}city;

typedef struct{
   medicine ** medicines;
   char * startCity;
   int originID;
   int destinationID;
   int distance;
   int medCount;
   unsigned companyID;
   unsigned planeID;
}plane;

typedef struct{
   plane ** companyPlanes;
   unsigned planesCount;
   unsigned ID;
}company;

typedef struct{
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
