#ifndef _VEC_H_
#define _VEC_H_

#include <stdlib.h>
#include <string.h>
#include <assert.h>

// vec: type* / size_t*
// (size_t)size, (size_t)capacity, (type)data...
//                                 ^ user pointer

#ifndef Vec_calloc
#define Vec_calloc calloc
#endif

#ifndef Vec_free
#define Vec_free free
#endif

#ifndef Vec_realloc
#define Vec_realloc realloc
#endif

#define Vec(T) T*

#define Vec_iter(T) T*

#define Vec_citer(T) const T*

#define Vec_new(T, ...) \
    ((Vec(T))__Vec_new( \
        sizeof((T[]){__VA_ARGS__}) / sizeof(T), \
        sizeof(T), \
        (T[]){__VA_ARGS__}))

#define Vec_del(vec) \
    Vec_free(__Vec_basePtr(vec))

#define Vec_capacity(vec) \
    ((vec) ? ((size_t*)(vec))[-1] : 0)

#define Vec_size(vec) \
    ((vec) ? ((size_t*)(vec))[-2] : 0)

#define Vec_at(vec, pos) \
    (assert((vec) && (pos) >= 0 && (pos) < Vec_size(vec)), \
     (vec)[(pos)])

#define Vec_clear(vec) \
    ((vec) ? __Vec_setSize((vec), 0) : 0)

#define Vec_push_back(vec, ...) \
    do { \
        if (Vec_size(vec) >= Vec_capacity(vec)) { \
            __Vec_grow((vec), __Vec_nextGrowSize(vec)); \
        } \
        (vec)[Vec_size(vec)] = (__VA_ARGS__); \
        __Vec_setSize(vec, Vec_size(vec) + 1); \
    }while(0)

#define Vec_pop_back(vec) \
    do { \
        if (!Vec_size(vec)) { break; } \
        __Vec_setSize(vec, Vec_size(vec) - 1); \
    }while(0)

#define Vec_reserve(vec, count) \
    do { \
        if (Vec_capacity(vec) < (count)) { \
            __Vec_grow((vec), (count)); \
        } \
    }while(0)

#define Vec_insert(vec, pos, begin, end, next) \
    do { \
        Vec(__typeof__(*(begin))) __vec_tmp = Vec_new(__typeof__(*(begin))); \
        for (__typeof__(begin) __vec_it = (begin); \
            __vec_it != (end); \
            next(__vec_it)) \
        { \
            Vec_push_back(__vec_tmp, *__vec_it); \
        } \
        Vec_reserve(vec, Vec_size(vec) + Vec_size(__vec_tmp)); \
        for (size_t __vec_i = 0; __vec_i < Vec_size(vec) - (pos); __vec_i++) { \
            (vec)[(pos)+Vec_size(__vec_tmp)+__vec_i] = (vec)[(pos)+__vec_i]; \
        } \
        for (size_t __vec_i = 0; __vec_i < Vec_size(__vec_tmp); __vec_i++) { \
            (vec)[(pos) + __vec_i] = __vec_tmp[__vec_i]; \
        } \
        __Vec_setSize(vec, Vec_size(vec) + Vec_size(__vec_tmp)); \
        Vec_del(__vec_tmp); \
    }while(0)

#define Vec_begin(vec) \
    (vec)

#define Vec_end(vec) \
    ((vec) ? &((vec)[Vec_size(vec)]) : NULL)

#define Vec_cbegin(vec) \
    ((const __Vec_type(vec)*)Vec_begin(vec))

#define Vec_cend(vec) \
    ((const __Vec_type(vec)*)Vec_end(vec))

#define Vec_iter_next(iter, ...) \
    ((iter)++)




#define __Vec_basePtr(vec) \
    ((vec) ? &((size_t*)(vec))[-2] : NULL)


#define __Vec_nextGrowSize(vec) \
    ((vec) ? Vec_capacity(vec) << 1 : 1)

#define __Vec_type(vec) __typeof__(*(vec))

#define __Vec_setSize(vec, size) \
    (__Vec_basePtr(vec)[0] = (size))

#define __Vec_setCapacity(vec, count) \
    (__Vec_basePtr(vec)[1] = (count))

#define __Vec_grow(vec, count) \
    do { \
        size_t __vec_count = (count); \
        size_t __vec_size = sizeof(size_t)*2 + \
            __vec_count*sizeof(__typeof__(*(vec))); \
        size_t* __vec_ptr; \
        if ((vec)) { \
            __vec_ptr = __Vec_basePtr(vec); \
            __vec_ptr = Vec_realloc(__vec_ptr, __vec_size); \
            assert(__vec_ptr); \
        }else{ \
            __vec_ptr = Vec_calloc(1, __vec_size); \
            assert(__vec_ptr); \
        } \
        (vec) = (__Vec_type(vec)*)(&__vec_ptr[2]); \
        __Vec_setCapacity(vec, __vec_count); \
    }while(0)

static inline void* __Vec_new(size_t count, size_t size, void* data){
    if (!count) return NULL;

    size_t vecsize = sizeof(size_t)*2 + size * count;
    size_t* v = calloc(1, vecsize);

    v[0] = v[1] = count;
    memcpy(v+2, data, size * count);

    return v+2;
}


#endif
