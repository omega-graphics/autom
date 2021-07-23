/**
 @file ADT.h

 Contains Base ADTs used in AUTOM
*/

#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#ifndef AUTOM_ADT_H
#define  AUTOM_ADT_H

namespace autom {


    #if __cplusplus > 201402L
    #define CONSTEXPR_CXX17 constexpr
    #else 
    #define CONSTEXPR_CXX17 
    #endif

    #define NOEXCEPT noexcept
    /// @brief Key Value Store (similar to an Unordered Map)
    template<class K,class V>
    class KVStore {
    public:
        typedef std::pair<K,V> Entry;

        typedef unsigned size_type;
        typedef K & key_reference;
        typedef V & value_reference;

    private:
        std::vector<Entry> body;
        size_type _size;

        inline void _insert(const Entry & entry){
            body.push_back(entry);
        };

        value_reference ref(key_reference k){
            for(auto & e : body){
                if(e.first == k){
                    return e.second;
                };
            };
            Entry e;
            e.first = k;
            body.push_back(e);
            return body.back().second;
        };


    public:
        typedef typename std::vector<Entry>::iterator iterator;
        typedef typename std::vector<Entry>::const_iterator const_iterator;

        iterator begin(){
            return body.begin();
        };

        iterator end(){
            return body.end();
        };

        const_iterator cbegin(){
            return body.cbegin();
        };

        const_iterator cend(){
            return body.cend();
        };
    private:

        inline iterator _find(key_reference k){
            auto it = body.begin();
            while(it != body.end()){
                if(it->first == k)
                    break;
                ++it;
            };
            return it;
        };

    public:

        void insert(Entry && entry){
            _insert(entry);
        };

        void insert(const Entry & entry){
            _insert(entry);
        };

        iterator find(key_reference k){
            return _find(k);
        };

        iterator find(K && k){
            return _find(k);
        };

        value_reference operator[](K && k){
            return ref(k);
        };

        value_reference operator[](key_reference k){
            return ref(k);
        };

    };

    /// @brief A constant reference to a String
    class StrRef {
        const char *_data;
    public:
        typedef unsigned size_type;
    private:
        const size_type _size;
    public:
        typedef const char *iterator;
        typedef const char &reference;

        CONSTEXPR_CXX17 const size_type & size() const NOEXCEPT{
            return _size;
        };
        CONSTEXPR_CXX17 const char *data() const NOEXCEPT{
            return _data;
        };

        iterator begin() const{
            return _data;
        };
        iterator end() const{
            return _data + _size;
        };

        CONSTEXPR_CXX17 bool compare(const char *data,size_type len) const{
            if(len != _size)
                return false;

            bool rc = true;
            
            for(unsigned i = 0;i < len;i++){
                auto & c = data[i];
                if(c != begin()[i]){
                    rc = false;
                    break;
                };
            };

            return rc;
        };

        CONSTEXPR_CXX17 bool operator==(const char *data) const{
            return compare(data,std::strlen(data));
        };

        CONSTEXPR_CXX17 bool operator!=(StrRef & other) const{
            return compare(other.data(),other.size());
        };

        StrRef(const char *c_str):_data(c_str),_size(std::strlen(c_str)){

        };

        StrRef(char *data,size_type len):_data(data),_size(len){

        };

        StrRef(std::string & str):_data(str.data()),_size(str.size()){

        };

    };
    template<class T,class ..._Args>
    auto unpackToArray(_Args && ...args) 
    -> std::array<T,sizeof...(args)> {
        return {T(args)...};
    };

};

#endif