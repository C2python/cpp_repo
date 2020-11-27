#ifndef _LRU_MAP_H
#define _LRU_MAP_H

#include <iostream>
#include <mutex>
#include <list>
#include <map>

template<typename K, typename V>
class lru_map{
private:
    struct entry{
        V value;
        typename std::list<K>::iterator lru_iter;
    };
    std::mutex mtx;
public:
    std::map<K,entry> entries;
    std::list<K> entries_lru;
    size_t max;

public:
    lru_map(int max):max(max){}
    virtual ~lru_map(){}

    bool find(const K& key, V *value);
    void add(const K& key, V& value);
    void erase(const K& key);
    void dump();
};

template<typename K,typename V>
bool lru_map<K,V>::find(const K& key,V *value){
    std::lock_guard<std::mutex> lock(mtx);
    typename std::map<K,entry>::iterator iter = entries.find(key);
    if (iter == entries.end()){
        return false;
    }
    entry& e = iter->second;
    if (value){
        *value = e.value;
    }
    entries_lru.erase(e.lru_iter);
    entries_lru.push_front(key);
    e.lru_iter = entries_lru.begin();
    return true;
}

template<typename K,typename V>
void lru_map<K,V>::add(const K& key, V& value){
    std::lock_guard<std::mutex> lock(mtx);
    typename std::map<K,entry>::iterator miter = entries.find(key);
    if (miter != entries.end()){
        entries_lru.erase(miter->second.lru_iter);
    }
    entries_lru.push_front(key);
    entry& e = entries[key];
    e.value = value;
    e.lru_iter = entries_lru.begin();

    if (entries_lru.size() > max){
        typename std::list<K>::reverse_iterator citer = entries_lru.rbegin();
        miter = entries.find(*citer);
        entries.erase(miter);
        entries_lru.pop_back();
    }
}


template<typename K,typename V>
void lru_map<K,V>::erase(const K& key){
    std::lock_guard<std::mutex> lock(mtx);
    typename std::map<K,entry>::iterator iter = entries.find(key);
    if (iter != entries.end()){
        entries_lru.erase(iter->second.lru_iter);
        entries.erase(iter);
    }
}

template<typename K,typename V>
void lru_map<K,V>::dump(){
    if (!entries_lru.empty()){
        std::cout<<"LRU_CACHE KEY SEQ"<<std::endl;;
        for (const auto& it:entries_lru){
            std::cout<<"Key: "<<it<<std::endl;
        }
        std::cout<<"MAP MEM"<<std::endl;
        for (const auto& it:entries){
            std::cout<<"key: "<<it.first<<",Value: "<<it.second.value<<std::endl;
        }
    }
}

#endif