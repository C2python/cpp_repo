/**
 * Provide abstract thread interface
 * 
 * 
**/

#ifndef UP_THREAD_HPP
#define UP_THREAD_HPP

#include <thread>
#include <memory>

class UPThread: public std::enable_shared_from_this<UPThread>{
public:
  std::thread::id t_id;
  //std::string t_name;

  std::thread handle;

public:

  UPThread()=default;
  UPThread(const UPThread&)=delete;
  UPThread& operator=(const UPThread&)=delete;
  UPThread(UPThread&&)=default;
  UPThread& operator=(UPThread&&)=default;

  void create(const char* name){
    //t_name = name;
    auto self = shared_from_this();
    handle = std::thread(&UPThread::entry,this,self);
  }

  virtual void run()=0;

  virtual void entry(std::shared_ptr<UPThread> self){
    t_id = std::this_thread::get_id();
    run();
  }

  void join(){
    if (handle.joinable())
      handle.join();
  }

  void detach(){
    handle.detach();
  }

protected:
  /**
   * In the derived class, use shared_from_base instead of shared_from_this 
  **/
  template <typename Derived>
  std::shared_ptr<Derived> shared_from_base()
  {
      return std::static_pointer_cast<Derived>(shared_from_this());
  }

};


#endif