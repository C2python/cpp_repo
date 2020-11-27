#include "prebufferstream.hpp"

#include <iostream>
#include <unistd.h>
#include <cstring>

void prebuffer::init(int fd, int om, bool throw_exception){
  m_filedes = fd;
  if (m_filedes > 0){
    m_mode = om;
  }
  if (throw_exception && !is_open()){
      throw FileError();
  }
  setg(buffer+s_pback_size,
       buffer+s_pback_size,
       buffer+s_pback_size);
  setp(buffer,
       buffer+s_buf_size-1);
}

prebuffer::prebuffer(int fd, int om, bool throw_exception){
  init(fd,om,throw_exception);
}

void prebuffer::close(){
  if (is_open()){
    this->sync();
    m_mode = 0;
    m_filedes = -1;
  }
}

prebuffer::int_type prebuffer::underflow(){

  if (gptr() < egptr()){
    return static_cast<unsigned char>(*gptr());
  }
  int num_putback=0;
  if (s_pback_size > 0){
    num_putback = gptr()-eback();
    if (num_putback > s_pback_size){
      num_putback = s_pback_size;
    }
    std::memcpy(buffer+s_pback_size-num_putback,gptr()-num_putback,num_putback);
  }

  const int num = ::read(m_filedes,buffer+s_pback_size,s_buf_size-s_pback_size);

  if (num <= 0){return EOF;}
  setg(buffer+s_pback_size-num_putback,buffer+s_pback_size,buffer+s_pback_size+num);

  return static_cast<unsigned char>(*gptr());

}

prebuffer::int_type prebuffer::overflow(int_type c){
  std::cout<<"Overflow Happend"<<std::endl;
  if (!(m_mode & std::ios::out) || !is_open()) return EOF;
  if (c != EOF){
    *pptr() = c;
    pbump(1);
  }
  if (flushoutput() == EOF)
    {
      return -1; 
    }

  return c;
}

int prebuffer::flushoutput(){
  if (!(m_mode & std::ios::out) || !is_open()) return EOF;
  size_t num = pptr()-pbase();
  if (::write(m_filedes,buffer,num) == -1){
    return EOF;
  }
  pbump(-num);
  return num;
}

int prebuffer::sync(){
  if (flushoutput() == EOF){
    return -1;
  }
  return 0;
}