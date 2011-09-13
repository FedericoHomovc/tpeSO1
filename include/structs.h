/***
***
***				structs.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

/*
 * name: medicine
 * description: stands for a package of certain medicine
 * @name: name of the medicine
 * @quantity: quantity of the medicine
 *
 */
typedef struct{
   char * name;
   int quantity;
}medicine;

/*
 * name: city
 * description: city representation
 * @name: name of the city
 * @medicines: array with the city medicine requirements
 * @ID: identification of the city
 * @medCount:lenght of the medicine array
 *
 */
typedef struct{
   char * name;
   medicine ** medicines;
   unsigned ID;
   unsigned medCount;
}city;

/*
 * name: plane
 * description: plane representation.
 * @medicines: array with the medicines stored in the plane.
 * @startCity: departure city of the plane.
 * @destinationID: ID of the city of destination.
 * @medCount:lenght of the medicine array.
 * @companyID: ID of the company of the plane.
 * @planeID: ID of the plane.
 */
typedef struct{
   medicine ** medicines;
   char * startCity;
   int destinationID;
   int distance;
   int medCount;
   unsigned companyID;
   unsigned planeID;
}plane;

/*
 * name: company
 * description: company representation
 * @companyPlanes: array of planes of the company.
 * @planesCount:plane array length.
 * @ID: ID of the company.
 *
 */
typedef struct{
   plane ** companyPlanes;
   unsigned planesCount;
   unsigned ID;
}company;

/*
 * name: map
 * description: map representation.
 * @cities: array of cities.
 * @graph: matrix with the distances from city to city.
 * @citiesCount: quantity of cities.
 * @companiesCount: quantity of companies.
 */
typedef struct{
   city ** cities;
   int ** graph;
   int citiesCount;
   int companiesCount;
}map;

