#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef WIDGET
#define WIDGET

// just a thing we can set to (almost) any size so we can pick any working set
// we're going to sort these by number, but the padding size is pickable (-DPAD_SIZE=N)
// Well, that would of course be naive :)
// Due to structure alignment (http://c-faq.com/struct/align.esr.html) it's most likely going to be bigger
typedef struct {
	uint32_t number;
	char padding[PAD_SIZE];
} widget;

#ifdef VERBOSE

// check if a list is sorted in ascending order
void is_sorted( widget* widgets, int from, int to );

void print_array( widget* widgets, int from, int to, int width );

#else

#define is_sorted(...) // nop

#define print_array(...) // nop

#endif


int write_widgets( widget* widgets, size_t count, const char* filename_out );

void contains_same_elements( widget* a, widget* b, size_t count);

size_t read_widgets( const char* filename, widget** widgets );

int compare_widget(const void* a, const void* b);

// returns a string representing the widget
// since we usually just want to print this, defer free() until later.
// Don't forget to call free_allocated_strings() later.
const char* sfw( char* is_a_widget );

#endif
