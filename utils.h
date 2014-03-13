#ifndef UTILS
#define UTILS

#include <stdint.h>

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )


#ifdef VERBOSE

int SUPPRESS_bak, SUPPRESS_new;

#define SUPPRESS_STDOUT \
		extern int SUPPRESS_bak, SUPPRESS_new; \
		fflush(stdout); \
		SUPPRESS_bak = dup(1); \
		SUPPRESS_new = open("/dev/null", O_WRONLY); \
		dup2(SUPPRESS_new, 1); \
		close(SUPPRESS_new);

#define RETURN_STDOUT \
		fflush(stdout); \
		dup2(SUPPRESS_bak, 1); \
		close(SUPPRESS_bak);

#else

#define SUPPRESS_STDOUT // nop
#define RETURN_STDOUT // nop

#endif


#ifdef VERBOSE
// just printf but not doing anthing if not VERBOSE
void say( const char* format, ... );

#else

#define say(...) // nop

#endif

// reads the Time Stamp Counter
// for getting CPU cycles
// http://en.wikipedia.org/wiki/Time_Stamp_Counter
// because	
uint64_t read_TSC();


/*
Conveniently append strings, with optional printf style formatting.
This makes sure the resulting string is big enough.
*/
char* append_str( char* prefix, const char* format, ... );

typedef struct allocated_string_node {
	const char* allocated_string;
	struct allocated_string_node* next;
} allocated_string_node;

void save_allocated_string( const char* str );
void free_allocated_strings();

#endif
