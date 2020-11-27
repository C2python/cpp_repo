#include <iostream>
#include "lru_map.hpp"

int main(int argc,char *argv[]){
    lru_map<int,char> lm(3);
    char a = 'a';
    char b = 'b';
    char c = 'c';
    char e = 'e';
    lm.add(2,b);
    lm.add(1,a);
    lm.add(5,e);
    lm.add(3,c);
    lm.dump();
    char* value = new char;
    lm.find(5,value);
    lm.dump();
    delete value;

}