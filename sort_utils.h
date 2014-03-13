/*
Shellsort the numbers using the Marcin Ciura sequence.

http://sun.aei.polsl.pl/~mciura/publikacje/shellsort.pdf

The numbers are sorted in place

Rather than supply a count this specifies a start and end making this suitable
for sorting subsections of an array and also for embedding it in another sort.

An alternative would be to pass a pointer to the offset and a count but I like this better.

This sorts the array numbers in ascending order from [start, end] inclusive. (end being inclusive is not very intuitive though)
*/

typedef int (*comparator)(const void* a, const void* b);

typedef void (*sorter)( void* base, size_t nel, size_t width, comparator compare );

// just a thing we can set to (almost) any size so we can pick any working set
// we're going to sort these by number, but the padding size is pickable (-DPAD_SIZE=N)
// Well, that would of course be naive :)
// Due to structure alignment (http://c-faq.com/struct/align.esr.html) it's most likely going to be bigger
typedef struct {
	uint32_t number;
	char padding[PAD_SIZE];
} widget;

void shellsort( void* base, size_t nel, size_t width, comparator compare );


// read a bunch of integers from a file and returns the count
// exits on failure to malloc, open the file etc.
// should maybe return negative numbers and set errno instead
size_t read_numbers( const char* filename, int** numbers );

size_t read_widgets( const char* filename, widget** widgets );

int write_numbers( int* numbers, size_t count, const char* filename_out );

int write_widgets( widget* widgets, size_t count, const char* filename_out );

void contains_same_elements( widget* a, widget* b, size_t count);

int compare_int(const void* a, const void* b);

int compare_widget(const void* a, const void* b);

