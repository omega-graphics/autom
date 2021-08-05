#include "Autom.h"
#include "Execution.h"
#include <cassert>

namespace autom {

    namespace eval {

        Object::Object(Ty type):type(type){

        };

        void Object::incRef(){
            refCount += 1;
        }

        void Object::decRec(){
            refCount -= 1;
        }

        Boolean::Boolean(bool & val):Object(Object::Boolean),data(val){

        }

        bool Boolean::value(){
            return data;
        };

        Boolean::~Boolean(){

        };

        String::String(std::string & val):Object(Object::String),data(val){

        }

        size_t String::length(){
            return data.length();
        }

        bool String::empty(){
            return length() == 0;
        };

        StrRef String::value() const {
            return {data};
        }

        String::~String(){

        };

        Array::Array(std::vector<Object *> & val):Object(Object::Array),data(val){

        }

        size_t Array::length(){
            return data.size();
        }

        bool Array::empty(){
            return length() == 0;
        };

        Array::Iterator Array::getBeginIterator(){
            return data.begin();
        }

        Array::Iterator Array::getEndIterator(){
            return data.end();
        }

        ArrayRef<Object *> Array::value() const{
            return {data};
        }

        std::vector<std::string> Array::toStringVector(){
            std::vector<std::string> vec;
            for(auto it = getBeginIterator();it != getEndIterator();it++){
                assert(objectIsString(*it) && "Object is not a string.. Cannot convert to String Array.");
                vec.push_back(castToString(*it)->value());
            };
            return vec;
        };

        Array::~Array(){

        }

        TargetWrapper::~TargetWrapper(){

        };

         Boolean * castToBool(Object *object){
            return ((eval::Boolean *)object);
        };

        String * castToString(Object *object){
            return ((eval::String *)object);
        };

        Array * castToArray(Object *object){
            return ((eval::Array *)object);
        };

    }

    typedef eval::Object Object;

     /// Typecheck if Object is Boolean

    bool objectIsBool(Object *object){
        return object->type == Object::Boolean;
    };

    /// Typecheck if Object is String

    bool objectIsString(Object *object){
        return object->type == Object::String;
    };

    /// Typecheck if Object is Array

    bool objectIsArray(Object *object){
        return object->type == Object::Array;
    };

    // / Get begin iterator of Array

    //  arrayIteratorBegin(Object *object){
    //    return objectToVector(object).begin();
    // };

    // /// Get begin iterator of Array

    //  arrayIteratorEnd(Object *object){
    //    return objectToVector(object).end();
    // };

    /// Get length of Array

    size_t arrayLength(Object *object){
        assert(objectIsArray(object));
        return ((eval::Array *)object)->length();
    };

    Object *toObject(bool & val){
        return new eval::Boolean(val);
    };

    Object *toObject(std::string &val){
        return new eval::String(val);
    };

    Object *toObject(std::vector<Object *> &val){
        return new eval::Array(val);
    };


    bool objectToBool(Object *object){
        assert(objectIsBool(object));
        return eval::castToBool(object)->value();
    };
    StrRef objectToString(Object *object){
        assert(objectIsString(object));
         return eval::castToString(object)->value();
    };

    ArrayRef<Object *> objectToVector(Object *object){
        assert(objectIsArray(object));
        return eval::castToArray(object)->value();
    };



}