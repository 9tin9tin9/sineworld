#ifndef _STR_H_
#define _STR_H_

#include <string.h>
#include <stdlib.h>
#include <assert.h>

// str: char* / size_t*
// (size_t)len(without trailing NUL), (size_t)capacity, (char)data...
//                                                      ^ user pointer

#ifndef Str_calloc
#define Str_calloc calloc
#endif

#ifndef Str_free
#define Str_free free
#endif

#ifndef Str_realloc
#define Str_realloc realloc
#endif

#define Str char*

#define Str_iter(_) char*

#define Str_citer(_) const char*

#define Str_new() ((char*)NULL)

#define Str_from(...) __Str_from(__VA_ARGS__)

#define Str_del(__str) \
    Str_free(__Str_basePtr(__str))

#define Str_capacity(__str) \
    ((__str) ? ((size_t*)(__str))[-1] : 0)

#define Str_len(__str) \
    ((__str) ? ((size_t*)(__str))[-2] : 0)

#define Str_at(__str, __pos) \
    (assert((__str) && (__pos) >= 0 && (__pos) < Str_len(__str)), \
     (__str)[(__pos)])

#define Str_push_back(__str, ...) \
    do { \
        if (Str_len(__str) + 1 >= Str_capacity(__str)) { \
            __Str_grow((__str), __Str_nextGrowSize(__str)); \
        } \
        size_t* __str_ptr = __Str_basePtr(__str); \
        (__str)[*__str_ptr] = (__VA_ARGS__); \
        (__str)[*__str_ptr + 1] = 0; \
        __str_ptr[0]++; \
    }while(0)

#define Str_pop_back(__str) \
    do { \
        if (!Str_len(__str)) { break; } \
        size_t* __str_ptr = __Str_basePtr(__str); \
        (__str)[*__str_ptr] = 0; \
        __str_ptr[0]--; \
    }while(0)

#define Str_reserve(__str, __count) \
    do { \
        if (Str_capacity(__str) < (__count)) { \
            __Str_grow((__str), (__count)); \
        } \
    }while(0)

#define Str_begin(__str) \
    (__str)

#define Str_end(__str) \
    ((__str) ? &((__str)[Str_len(__str)]) : NULL)

#define Str_cbegin(__str) \
    ((const char*)Str_begin(__str))

#define Str_cend(__str) \
    ((const char*)Str_end(__str))

#define Str_iter_next(__iter, ...) \
    ((__iter)++)

#define Str_copy(__str, __from) \
    do { \
        size_t __str_size = strlen((__from)) + 1; \
        Str_reserve(__str, __str_size); \
        strlcpy((__str), (__from), __str_size); \
        __Str_basePtr(__str)[0] = __str_size - 1; \
    }while(0)

#define Str_append(__str, __src) \
    do { \
        size_t __str_size = Str_len(__str) + strlen((__src)) + 1; \
        Str_reserve(__str, __str_size); \
        strlcat((__str), (__src), __str_size); \
        __Str_basePtr(__str)[0] = __str_size - 1; \
    }while(0)



#define __Str_basePtr(__str) \
    ((__str) ? &((size_t*)(__str))[-2] : NULL)

#define __Str_type(_) char

#define __Str_nextGrowSize(__str) \
    ((__str) ? Str_capacity(__str) << 1 : 1)

#define __Str_grow(__str, __count) \
    do { \
        size_t __str_count = (__count); \
        size_t __str_size = sizeof(size_t)*2 + __str_count+sizeof(char); \
        size_t* __str_ptr; \
        if ((__str)) { \
            __str_ptr = __Str_basePtr(__str); \
            __str_ptr = Str_realloc(__str_ptr, __str_size); \
            assert(__str_ptr); \
        }else{ \
            __str_ptr = Str_calloc(1, __str_size); \
            assert(__str_ptr); \
        } \
        __str_ptr[1] = __str_count; \
        (__str) = (char*)(&__str_ptr[2]); \
    }while(0)

static inline char* __Str_from(char* src){
    Str s = Str_new();
    Str_copy(s, src);
    return s;
}

#endif
