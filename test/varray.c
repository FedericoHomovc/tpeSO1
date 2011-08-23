/*
 * Includes
 */

#include <stdlib.h>
#include <stdio.h>
#include "../include/varray.h"


/*
 * Type definitions
 */

/* A structure representing a vArray */
typedef struct vArrStruct
{
	void ** array;
	int allocated;
	int used;
} vArrStruct;


/*
 * Static functions
 */


static int reallocArray(vArray array)
{
	if(array->allocated <= array->used)
	{
		void ** temp = realloc(array->array,
				sizeof(void *) * array->allocated * 2);
		if(temp == NULL)
		{
			/* Fatal problem */
			return ERR_VARRAY_NOMEM;
		}
		else
		{
			array->allocated *= 2;
			array->array = temp;
		}
	}
	return 0;
}


/*
 * Functions
 */

/* Initializes the array. */
vArray vArray_init(int initialSize)
{
	vArray vArr;
	int i;
	void ** arr = malloc(sizeof(void *) * initialSize);
	if(arr == NULL)
	{
		return NULL;
	}

	vArr = malloc(sizeof(struct vArrStruct));
	if(vArr == NULL)
	{
		free(arr);
		return NULL;
	}

	vArr->allocated = initialSize;
	vArr->used = 0;
	vArr->array = arr;

	for(i = 0; i < initialSize; i++)
	{
		vArr->array[i] = NULL;
	}

	return vArr;
}

/* Destroys the array. */
void vArray_destroy(vArray array)
{
	free(array->array);
	free(array);
}

/* Gets the element stored at a position. */
void * vArray_getAt(vArray array, int pos)
{
	if (array == NULL || array->used <= pos || pos < 0)
		return NULL;
	return array->array[pos];
}

/* Puts the specified element in the specified position. */
void vArray_putAt(vArray array, void * elem, int pos)
{
	if (array == NULL || elem == NULL || pos < 0 || array->used <= pos)
		return;
	array->array[pos] = elem;
}

/* Inserts the element at the end of the array. */
int vArray_insertAtEnd(vArray array, void * elem)
{
	if(array == NULL || elem == NULL)
		return 0;
	array->array[array->used] = elem;
	array->used++;
	return reallocArray(array);
}

/* Removes the element at the end of the array (sets it to NULL). */
void vArray_removeAtEnd(vArray array)
{
	array->array[--(array->used)] = NULL;
}


/* Removes an element at a specific position. */
int vArray_removeAt(vArray array, int index)
{
	int i;

	if(array == NULL || index > array->used - 1 || array->used == 0)
		return ERR_VARRAY_INVALID;

	for(i = index; i < array->used - 1; i++)
		array->array[i] = array->array[i+1];

	(array->used)--;

	return 1;
}

/* Returns the amount of current elements in the array. */
int vArray_getSize(vArray array)
{
	return array->used;
}

/* Searches a vArray. */
void * vArray_search(vArray array, int (*compare)(void *, void*), void * searchedElem)
{
	int i;
	if(array == NULL)
		return NULL;
	for (i = 0; i < array->used; i++)
	{
		if((*compare)(searchedElem, array->array[i]) == 0)
			return array->array[i];
	}
	return NULL;
}
