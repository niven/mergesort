#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"

#ifdef VERBOSE	

void say( const char* format, ... ) {
	va_list arglist;
	va_start( arglist, format );
	vprintf( format, arglist );
	va_end( arglist );
}

#endif	

/********** linked list of char* so we can just make strings and save them for free()ing later ***********/

static allocated_string_node* _root_allocated_strings = NULL;
static allocated_string_node* _tail_allocated_strings = NULL;

void save_allocated_string( const char* str ) {
	
	if( _tail_allocated_strings == NULL ) {
		_root_allocated_strings = (allocated_string_node*)malloc( sizeof(allocated_string_node) );
		if( _root_allocated_strings == NULL ) {
			perror("malloc()");
			exit( EXIT_FAILURE );
		}

		_tail_allocated_strings = _root_allocated_strings;
		_tail_allocated_strings->next = NULL;
		_tail_allocated_strings->allocated_string = str;
	} else {
		_tail_allocated_strings->next = (allocated_string_node*)malloc( sizeof(allocated_string_node) );
		if( _tail_allocated_strings->next == NULL ) {
			perror("malloc()");
			exit( EXIT_FAILURE );
		}
		_tail_allocated_strings->next->allocated_string = str;
		_tail_allocated_strings = _tail_allocated_strings->next;
		_tail_allocated_strings->next = NULL;
	}
	
}

void free_allocated_strings() {
	
	allocated_string_node* temp;
	while( _root_allocated_strings != NULL ) {
		temp = _root_allocated_strings;

		_root_allocated_strings = _root_allocated_strings->next;

		say("free_allocated_strings: freeing [%s]\n", temp->allocated_string );
		free( (void*)temp->allocated_string );
		free( temp );
		
	}
	
}

/********** linked list of char* so we can just make strings and save them for free()ing later ***********/



/*
 assembly code to read the TSC 
 (Time Stamp Counter)
*/
uint64_t read_TSC() {
  unsigned int hi, lo;
  __asm__ ("cpuid");
  __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}

char* append_str( char* prefix, const char* format, ... ) {
	
	char* formatted_string;
	
	va_list argslist;
	va_start( argslist, format );
	vasprintf( &formatted_string, format, argslist );
	va_end( argslist );
	
	size_t len = strlen( formatted_string );
	char* temp = realloc( prefix, strlen(prefix) + len + 1 ); // length of both + NUL
	if( temp == NULL ) {
		free( prefix );
		perror("realloc()");
		exit( EXIT_FAILURE );
	}
	
	temp = strcat( temp, formatted_string );
	free( formatted_string );
	
	return temp;
}


