#include <stddef.h>
#include <stdint.h>

#ifndef AUTOM_UTIL_H
#define AUTOM_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

void *autom_alloc_with_data(void *data,size_t size);
void autom_free_ptr(void *ptr);


#ifdef __cplusplus
}

typedef struct {
    void *data;
}  AutomObjectCWrapper;

AutomObjectCWrapper *autom_new_object(void *data);
void autom_free_object(void *ptr);

#endif


#endif