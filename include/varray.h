

#ifndef VARRAY_H_
#define VARRAY_H_


/*
 * Macros and constants
 */

/* Not enough memory to do the requested operation */
#define ERR_VARRAY_NOMEM -5413
/* Invalid operation */
#define ERR_VARRAY_INVALID -1


/*
 * Type definitions
 */

/* A variable length array abstract data type. */
typedef struct vArrStruct * vArray;


/*
 * Functions
 */


vArray vArray_init(int initialSize);

void vArray_destroy(vArray array);

void * vArray_getAt(vArray array, int pos);

void vArray_putAt(vArray array, void * elem, int pos);

int vArray_insertAtEnd(vArray array, void * elem);

void vArray_removeAtEnd(vArray array);

int vArray_removeAt(vArray array, int index);

int vArray_getSize(vArray array);

void * vArray_search(vArray array, int (*compare)(void *, void *),
		void * searchedElem);

#endif /* VARRAY_H_ */
