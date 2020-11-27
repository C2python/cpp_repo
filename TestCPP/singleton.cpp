#include<iostream>
using namespace std;


class singleton{
public:
static singleton* getInstance(){
  if(instance == nullptr){
     instance = new singleton;
  }
  return instance;
}

private: 
static singleton* instance;
singleton(){}
~singleton(){}
singleton(const singleton& rhs)=delete;
singleton& operator=(const singleton& rhs)=delete;
singleton(singleton&& rhs)=delete;
singleton& operator=(singleton&& rhs)=delete;
};

singleton* singleton::instance=nullptr;

int main(int argc,char *argv[]){
   singleton* ptr = singleton::getInstance();
}
