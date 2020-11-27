#include "prebufferstream.hpp"
#include <fcntl.h> 
#include <ostream>
#include <istream>
#include <unistd.h>
#include <iostream>

int main(int argc,char* argv[]){

  std::string file_name1 = "test";
  std::string file_name2 = "intest";
  int fdout;
  int fdin;
  if ((fdout=open(file_name1.c_str(),O_RDWR | O_CREAT | O_APPEND)) < 0){
    std::cout<<"ERROR Open File"<<std::endl;
  }
  if ((fdin=open(file_name2.c_str(),O_RDONLY | O_CREAT | O_APPEND)) < 0){
    std::cout<<"ERROR Open File"<<std::endl;
  }


  prebuffer ot{fdout,std::ios::out};
  prebuffer it{fdin,std::ios::in};
  std::ostream dout(&ot);
  std::istream dint(&it);
  dout<<1<<2<<3<<4<<5<<"\n";

  std::cout<<ot.get_str()<<std::endl;

  ot.sync();
  int line;
  while (dint>>line){
    std::cout<<line<<" ";
  }

  close(fdout);
  close(fdin);

  return 0;
}