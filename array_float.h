#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef struct {
	float* items;
	int used;
	int length;
} FloatArray;

typedef void (*FloatArrayForeachFunction)(float, int, FloatArray);
typedef float (*FloatArrayMapFunction)(float, int, FloatArray);
typedef int (*FloatArrayFilterFunction)(float, int, FloatArray);
typedef void (*FloatArraySetFunction) (float, int);
typedef float (*FloatArrayGetFunction) (int);
typedef void (*FloatArraySortFunction)(FloatArray, FloatArrayGetFunction, FloatArraySetFunction, ArraySwapFunction);


FloatArray FloatArray_create (int length) {
	// Allocates [length] * sizeof(float) bytes of memory and points array.items to it
	FloatArray array;
	array.length = length;
	array.used = 0;
	array.items = malloc(length * sizeof(float));
	if (array.items == NULL) {
		Array_error(MEM_ALLOC_FAIL);
		return array;	
	}
	for (int i = 0; i < array.length; i++) {
		array.items[i] = 0;
	}
	return array;	
}

// Creates a new FloatArray from an existing array (float elements[])
FloatArray Array_from_float (float* elements, int length) {
	FloatArray from = FloatArray_create(length);
	for (int i = 0; i < length; i++) {
		from.items[i] = elements[i];
	}
	from.used = length;
	return from;
}

// Resizes the array by allocating/deallocating memory
FloatArray Array_resize_float (FloatArray array, int resize_factor) {

	// resize_factor > 0 -> allocates more memory to array;
	// resize_factor < 0 -> deallocates some memory from array;

	array.items = realloc(array.items, (array.length + resize_factor) * sizeof(float));
	if (array.items == NULL) {
		Array_error(MEM_REALLOC_FAIL);
		return array;	
	}
	if (resize_factor > 0) {
		for (int i = array.used; i < array.length; i++) {
			array.items[i] = 0;
		}	
	}

	array.length += resize_factor;
	return array;
}


void Array_destroy_float (FloatArray array) {
	free(array.items);
}

// Joins a FloatArray to an existing FloatArray
FloatArray Array_concat_arr_float (FloatArray array, FloatArray concat, int length) {
	int prev_length = array.length;
	int prev_used = array.used;
	if (prev_used + concat.length > array.length) {
		int resize_factor = prev_used + concat.length - array.length;
		array = Array_resize_float(array, resize_factor); 
	}
	for (int i = 0; i < concat.length; i++) {
		array.items[prev_used + i] = concat.items[i];
	}
	array.used = prev_used + concat.used;
	return array;
}

// Joins an array of floats (float concat[]) to existing FloatArray
FloatArray Array_concat_float (FloatArray array, float* concat, int length) {
	int prev_used = array.used;
	if (prev_used + length > array.length) {
		int resize_factor = prev_used + length - array.length;
		array = Array_resize_float(array, resize_factor); 
	}
	for (int i = 0; i < length; i++) {
		array.items[prev_used + i] = concat[i];
	}
	array.used = prev_used + length;
	return array;
}

/* Loops over the array and runs a callback with each turn
   Callback receives the current element and the current index */
void Array_foreach_float (FloatArray array, FloatArrayForeachFunction function) {
	for (int i = 0; i < array.used; i++) {
		function(array.items[i], i, array);
	}
}

// Adds an element to the end of the array
FloatArray Array_push_float (FloatArray array, float element) {
	int insert_index = array.used;
	if (array.length <= array.used) {
		array = Array_resize_float(array, 1);
		insert_index = array.length - 1;
	}
	array.items[insert_index] = element;
	array.used++;
	return array;
}

// Adds an element to the start of the array
FloatArray Array_unshift_float (FloatArray array, float element) {
	if (array.used == array.length) {
		array = Array_resize_float(array, 1);	
	}
	for (int i = array.used - 1; i >= 0; i--) {
		float temp = array.items[i + 1];
		array.items[i + 1] = array.items[i];
		array.items[i] = temp;
	}
	array.items[0] = element;
	array.used++;
	return array;

}

// Removes an element from the end of the array
FloatArray Array_pop_float (FloatArray array) {
	array.used--;
	return array;
}

// Checks if element exists in array
int Array_exists_float (FloatArray array, float element) {
	int found = 0;
	for (int i = 0; i < array.used; i++) {
		if (array.items[i] == element) {
			found = 1;
			break;
		}
	}
	return found;
}

/* Loops over the array and runs a callback with each turn
   If callback returns 1, keeps the element in the array. Otherwise, removes it.
   This function does not alter the original array. */
FloatArray Array_filter_float (FloatArray array, FloatArrayFilterFunction function) {
	FloatArray filtered = FloatArray_create(0);
	for (int i = 0; i < array.used; i++) {
		int should_add = function(array.items[i], i, array);
		if (should_add) filtered = Array_push_float(filtered, array.items[i]);
	}
	return filtered;
}

/* Loops over the array and runs a callback with each turn
   Callback is given current element and can make changes to it.
   Callback is expected to return an float.
   This function does not alter the original array. */
FloatArray Array_map_float (FloatArray array, FloatArrayMapFunction function) {
	FloatArray mapped = FloatArray_create(0);
		for (int i = 0; i < array.used; i++) {
		float new_el = function(array.items[i], i, array);
		mapped = Array_push_float(mapped, new_el);
	}
	return mapped;
}

// Makes an exact copy of an FloatArray and returns it
FloatArray Array_copy_float (FloatArray array) {
	int filter (float el, int i, FloatArray arr) {
		return 1;
	}
	FloatArray copied = Array_filter_float(array, filter);
	copied.used = array.used;
	return copied;
}

// Return element at specified index from FloatArray
float Array_get_float (FloatArray array, int index) {
	if (!(index < array.used) || index < 0) {
		Array_error(UNUSED_ACCESS);
		return 0;
	}
	return array.items[index];
}

/* Sets the element at specified index from FloatArray to specified value.
   Returns an error if the array is not long enough. */
FloatArray Array_set_float (FloatArray array, float element, int index) {
	if (!(index < array.used)) {
		Array_error(UNUSED_ACCESS);
		return array;
	}
	else if (!(index < array.length) || index < 0) {
		Array_error(OUT_OF_BOUNDS);
		return array;
	}
	else {
		array.items[index] = element;
	}
	return array;
}

// Counts the number of occurences of specified element in IntArray
int Array_count_float (FloatArray array, float element) {
	int count = 0;
	for (int i = 0; i < array.used; i++) {
		if (array.items[i] == element) count++;
	}
	return count;
}

// Returns subarray made from IntArray from start to end indices (both included)
FloatArray Array_slice_float (FloatArray array, int start, int end) {
	if (start < 0 || !(end < array.used)) {
		Array_error(OUT_OF_BOUNDS);
	}
	FloatArray sliced = FloatArray_create(end - start + 1);
	for (int i = 0; i <= sliced.length; i++) {
		sliced.items[i] = array.items[i + start]; 
	}
	sliced.used = sliced.length;
	return sliced;
}

// Deletes an element from the array. Alters the oriinal array.
FloatArray Array_delete_float (FloatArray array, int index) {
	if (!(index < array.length) || index < 0) {
		Array_error(OUT_OF_BOUNDS);
		return array;
	}
	int filter (float el, int i, FloatArray arr) {
		return i != index;
	}
	array = Array_filter_float(array, filter);
	return array;
}

/* Insert element at specified index in IntArray.
   If element already exists, it is shifted forward and new element is placed at the index.
   If array isn't long enough, extends the array and places the new element at given index. */
FloatArray Array_insert_float (FloatArray array, float element, int index) {
	if (index < 0) {
		Array_error(OUT_OF_BOUNDS);
		return array;
	}
	FloatArray inserted = Array_copy_float(array);
	if (index > inserted.length - 1) {
		inserted = Array_resize_float(inserted, index - inserted.length + 1);
		inserted.used = index + 1;
	}
	else {
		inserted.used++;
	}
	for (int i = index; i < inserted.used; i++) {
		inserted.items[i + 1] = array.items[i];
	}
	inserted.items[index] = element;
	return inserted;
}

/* A generalised sort function. Accepts a sorter function.
   Provides helper functions for getting, setting and swapping elements in the array
   Does not alter the original array. */
FloatArray Array_sort_float (FloatArray array, FloatArraySortFunction function) {
	FloatArray sorted = FloatArray_create(array.length);
	sorted.used = array.used;
	for (int i = 0; i < sorted.length; i++) {
		sorted.items[i] = array.items[i];
	}
	float get (int index) {
		return Array_get_float(sorted, index);
	}
	void set (float element, int index) {
		sorted = Array_set_float(sorted, element, index);	
	}
	void swap (int index1, int index2) {
		float temp = sorted.items[index1];
		sorted.items[index1] = sorted.items[index2];
		sorted.items[index2] = temp;
	}
	function(sorted, get, set, swap);
	return sorted;
}

/* PREDEFINED HELPER FUNCTIONS
   These function supplement the main functions of the library. They include pre-written
   function for printing (using Array_foreach), bubble/selection/insertion
   sorts (using Array_sort) */

void FLOAT_ARRAY_PRINT (float el, int index, FloatArray arr) {
	printf("(%d): %f\n", index, el);
}

void FLOAT_ARRAY_BUBBLE_SORTER (FloatArray sorted, FloatArrayGetFunction get, FloatArraySetFunction set, ArraySwapFunction swap) {
	for (int i = 0; i < sorted.used; i++) {
		int swaps = 0;		
		for (int j = 0; j < sorted.used - i; j++) {
			if (j != 0) {
				if (get(j) < get(j - 1)) {
					swaps++;
					swap(j, j - 1);
				}
			} 
		}
		if (swaps == 0) break;
	}
}

void FLOAT_ARRAY_SELECTION_SORTER (FloatArray sorted, FloatArrayGetFunction get, FloatArraySetFunction set, ArraySwapFunction swap) {
	for (int i = 0; i < sorted.used - 1; i++) {
		int min_index = i;
		for (int j = i; j < sorted.used; j++) {
			if (get(j) < get(min_index)) min_index = j;
		}
		if (min_index == i) break;
		swap(i, min_index);
	}
}

void FLOAT_ARRAY_INSERTION_SORTER (FloatArray sorted, FloatArrayGetFunction get, FloatArraySetFunction set, ArraySwapFunction swap) {
		for (int i = 1; i < sorted.used; i++) {
		int j = i - 1;
		while (j >= 0 && get(j) < get(j - 1)) {
			swap(j, j - 1);
			j--;
		}
	}
}