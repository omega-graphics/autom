/**
 @file ADT.h

 Contains Base ADTs used in AUTOM
*/

#include <cstring>
#include <string>
#include <type_traits>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>

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
        typedef size_t size_type;
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

        CONSTEXPR_CXX17 bool operator==(std::string & other) const{
            return compare(other.c_str(),other.length());
        };
        
        CONSTEXPR_CXX17 bool operator==(const StrRef & other) const{
            return compare(other.data(),other.size());
        };

        CONSTEXPR_CXX17 bool operator!=(const StrRef & other) const{
            return !compare(other.data(),other.size());
        };

        StrRef():_data(nullptr),_size(0){

        };

        StrRef(const char *c_str):_data(c_str),_size(std::strlen(c_str)){

        };

        StrRef(char *data,size_type len):_data(data),_size(len){

        };

        StrRef(std::string & str):_data(str.data()),_size(str.size()){

        };

        StrRef(const std::string & str):_data(str.data()),_size(str.size()){

        };
        
        StrRef(const StrRef & other):_data(other._data),_size(other._size){
            
        };

        operator std::string() const{
            return {begin(),end()};
        }; 

    };


    std::ostream & operator<<(std::ostream & os,const StrRef & strRef);
    std::ostream & operator<<(std::ostream & os,StrRef & strRef);

    /// @brief A constant reference to an Array (std::vector or std::array)
    template<class T>
    class ArrayRef {
        const T *_data;
    public:
        typedef size_t size_type;
    private:
        const size_type _size;
    public:
        typedef const T *iterator;
        typedef const T &reference;

        CONSTEXPR_CXX17 const size_type & size() const NOEXCEPT{
            return _size;
        };
        
        CONSTEXPR_CXX17 const bool empty()const NOEXCEPT {
            return size() == 0;
        }
        
        CONSTEXPR_CXX17 const T *data() const NOEXCEPT{
            return _data;
        };

        iterator begin() const{
            return _data;
        };
        iterator end() const{
            return _data + _size;
        };

        reference front() const {
            return begin()[0];
        };

        reference back() const {
            return end()[-1];
        };

        template<class _It>
        ArrayRef(_It begin,_It end):_data(begin),_size(end - begin){

        };

        ArrayRef(T *data,size_type len):_data(data),_size(len){

        };

        ArrayRef(std::vector<T> & d):_data(d.data()),_size(d.size()){

        };

        ArrayRef(const std::vector<T> & d):_data(d.data()),_size(d.size()){

        };
        template<size_t length>
        ArrayRef(std::array<T,length> & d):_data(d.data()),_size(length){

        };

        reference operator[](size_type i) const{
            return begin()[i];
        };

        operator std::vector<T>(){
            return {begin(),end()};
        }; 

    };

    template<class K,class V>
    class MapRef {
        const std::pair<K,V> * _data;
    public:
        typedef unsigned size_type;
    private:
        const size_type _size;
    public:
        std::pair<K,V> *data() const{
            return _data;
        };
        size_type size()const {
            return _size;
        };
        typedef const std::pair<K,V> * iterator;
        typedef const K & key_reference;
        typedef const V & value_reference;
        iterator begin()const {
            return _data;
        };
        iterator end(){
            return _data + _size;
        };
        value_reference operator[](key_reference key){
            auto it = begin();
            while(it != end()){
                if(it->first == key){
                    return it->second;
                };
                ++it;
            };
            return begin()->second;
        };
        
//        MapRef(const std::pair<K,V> * _data,size_t length):_data(_data),_size(length){
//
//        };
        template<class InputIterator>
        MapRef(InputIterator _beg,InputIterator _end):_data(_beg),_size(_end - _beg){

        };
    };


    bool locateProgram(autom::StrRef prog,std::string path,std::string & out);
    
    
    class SHA256Hash {
        
        void *data;
        
        unsigned char * getResult();
        
    public:
        
        SHA256Hash();
        
        void addData(void *data,size_t dataSize);
        
        void getResultAsHex(std::string & out);
        
        ~SHA256Hash();
    };

};

#endif
