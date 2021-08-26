#include "Autom.h"
#include "Execution.h"
#include <cassert>

#include "Tokens.def"

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
    
        Boolean::Boolean(Boolean *other):Object(Object::Boolean){
            assign(other);
        }
    
        void Boolean::assign(Boolean * other){
            data = other->data;
        }
    
        Object *Boolean::performOperand(Object *rhs,autom::StrRef operand){
            auto other = castToBool(rhs);
            if(operand == OP_EQUALEQUAL){
                bool b = data == other->data;
                return new eval::Boolean(b);
            }
            else if(operand == OP_EQUALEQUAL_NOT){
                bool b = data != other->data;
                return new eval::Boolean(b);
            }
        }

        bool Boolean::value(){
            return data;
        }

        Boolean::~Boolean(){

        }
    
    
    
        

        String::String(std::string & val):Object(Object::String),data(val){

        }
        
        String::String(String * other):Object(Object::String){
            assign(other);
        }

        String::String():Object(Object::String),data(){

        }

        void String::assign(String *other) {
            data = other->data;
        }

        void String::assign(const std::string& string) {
            data = string;
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
        
        Object *String::performOperand(Object *rhs, autom::StrRef operand){
            auto other = castToString(rhs);
            
            if(operand == OP_EQUALEQUAL){
                bool b = data == other->data;
                return new eval::Boolean(b);
            }
            else if(operand == OP_EQUALEQUAL_NOT){
                bool b = data != other->data;
                return new eval::Boolean(b);
            }
            else if(operand == OP_PLUS){
                std::string newString = data + other->data;
                return new eval::String(newString);
            }
            else if(operand == OP_PLUSEQUAL){
                data += other->data;
                return this;
            }
        }

        String::~String(){

        }



        Array::Array(std::vector<Object *> & val):Object(Object::Array),data(val){

        }
    
        Array::Array(Array *other):Object(Object::Array){
            assign(other);
        };
    
        void Array::assign(Array *other){
            if(!data.empty()){
                for(auto o : data){
                    delete o;
                }
            }
            for(auto o : other->data){
                Object *obj;
                switch (o->type) {
                    case Object::String: {
                        obj = new eval::String(castToString(o));
                        break;
                    }
                    case Object::Boolean : {
                        obj = new eval::Boolean(castToBool(o));
                        break;
                    }
                    case Object::Array : {
                        obj = new eval::Array(castToArray(o));
                        break;
                    }
                    case Object::Target : {
                        // NOTE: TargetWrapper objects cannot be duplicated therefore the reference to the object is copied.
                        obj = o;
                        break;
                    }
                }
                data.push_back(obj);
            };
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
    
        Object * Array::performOperand(Object *rhs, autom::StrRef operand){
            assert(objectIsArray(rhs));
            auto other = castToArray(rhs);
            
            if(operand == OP_PLUS){
                auto newArray = new Array(this);
                newArray->data.resize(data.size() + other->data.size());
                auto otherArray = new Array(other);
                std::move(otherArray->data.begin(),otherArray->data.end(),newArray->getBeginIterator() + newArray->length() - 1);
                otherArray->data.resize(0);
                delete otherArray;
                return newArray;
            }
            else if(operand == OP_PLUSEQUAL){
                auto otherArray = new Array(other);
                data.resize(data.size() + otherArray->data.size());
                std::move(other->data.begin(),other->data.end(),data.begin() + data.size() - 1);
                otherArray->data.resize(0);
                delete otherArray;
                return this;
            }
            
            return this;
        }

        Array::Array():Object(Object::Array),data(){

        }

        Array::~Array(){
//            if(!data.empty())
//                for(auto o : data){
//                    if(o != NULL || o != nullptr)
//                        delete o;
//                }
        }

        Target * TargetWrapper::value() const {
            return t;
        };

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
