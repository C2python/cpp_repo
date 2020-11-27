/*  Author: zhangwen1@unionpay.com */
/*  Date: 2020-11-24               */

/*
 * Provide Custom Streambufeer Example 
*/ 

#ifndef PREBUFFERSTREAM_HPP
#define PREBUFFERSTREAM_HPP

#include <streambuf>
#include <cstdio>
#include <exception>

class prebuffer: public std::streambuf{

public:
  typedef std::char_traits<char> traits_ty;
  typedef traits_ty::int_type int_type;
  typedef traits_ty::pos_type pos_type;
  typedef traits_ty::off_type off_type;

private:
  int m_mode;
  int m_filedes;

  static const int s_buf_size = 4092;
  static const int s_pback_size = 4;
  char buffer[s_buf_size];

  void init(int fd, int om, bool throw_exception);

  int flushoutput();

public:

  prebuffer(int fd, int om, bool throw_exception=false);
  prebuffer(const prebuffer&)=delete;
  prebuffer& operator=(const prebuffer&)=delete;
  prebuffer(prebuffer&&)=delete;
  prebuffer& operator=(prebuffer&&)=delete;

  ~prebuffer(){close();}

  virtual int_type overflow(int_type c) override;
  virtual int_type underflow() override;

  bool is_open() const { return m_filedes >= 0; }

  void close();
  virtual int sync() override;

  std::string get_str(){
    return std::string(buffer,this->pptr()-buffer);
  }

};


class FileError: public std::exception{
  const char* what(void) const throw(){
    return "Invaild fd";
  }
};

#endif