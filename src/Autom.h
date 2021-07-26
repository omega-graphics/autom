#include <vector>
#include <string>

#ifndef AUTOM_AUTOM_H
#define  AUTOM_AUTOM_H


namespace autom {
    namespace eval {
        struct Object;
    }
    typedef eval::Object Object;

    bool objectIsBool(Object *object);
    bool objectIsString(Object *object);
    bool objectIsArray(Object *object);

    Object *toObject(bool & val);
    Object *toObject(std::string &val);
    Object *toObject(std::vector<Object *> &val);

    bool & objectToBool(Object *object);
    std::string & objectToString(Object *object);
    std::vector<Object *> objectToVector(Object *object);
    

    typedef const char *CString;

    typedef Object *(NativeFunc)(unsigned c,Object **object);

    #define AUTOM_NATIVE_FUNC(name) Object * name(unsigned c,Object **object)

    typedef NativeFunc *NativeFuncRef;



    struct Function {
        CString name;
        NativeFuncRef func;
    };

    struct Extension {
        void *libData;
        CString name;
        std::vector<Function> funcs;
    };
    typedef Extension *(*AutomExtEntryFunc)();
}
#define AUTOM_EXT_ENTRY nativeExtMain
#define STR_WRAP(d) #d

#define AUTOM_EXT_INIT() Extension * AUTOM_EXT_ENTRY()


#endif