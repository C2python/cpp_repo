#include "upthread.hpp"
#include <iostream>

class worker:public UPThread{
  void run() override{
    std::cout<<"Worker"<<std::endl;
  }
};

int main(int argc,char* argv[]){
  std::shared_ptr<worker> ptr = std::make_shared<worker>();
  ptr->create("Worker");
  ptr->join();
}