#include "autom_util.h"
#include <new>
#include <cstdlib>
#include <cstring>

extern "C" {

void *autom_alloc_with_data(void *data,size_t size){
    auto d = malloc(size);
    memmove(d,data,size);
    return d;
};

void autom_free_ptr(void *ptr){
    free(ptr);
};

}

AutomObjectCWrapper *autom_new_object(void *data){
    auto _ptr = new AutomObjectCWrapper;
    _ptr->data = data;
    return _ptr;
}

void autom_free_object(void *ptr){
    auto _data = (AutomObjectCWrapper *)ptr;
    delete _data;
}