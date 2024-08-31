/* overloading based on number of arguments */

#define OVERLOAD_2(_1, _2, NAME, ...) NAME

/* example usecase */

typedef struct { /* ... */ } arr_t;

arr_t *arr_make_allocator(unsigned members, void *(*alloc)(unsigned long));
arr_t *arr_make_default(unsigned members);

#define arr_make(...) \
	OVERLOAD_2(__VA_ARGS__, arr_make_allocator, arr_make_default)(__VA_ARGS__)

/* and then in your sources... */

#include <alloca.h>

static inline void func(void) {
	arr_t *on_heap = arr_make(128);
	/* or */
	arr_t *on_stack = arr_make(128, alloca);
}
