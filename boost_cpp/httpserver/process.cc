#include "process.hpp"

int putobj(ClientIO* client,boost::asio::yield_context yield){
    client->send_100_continue();
    int accept_len = 0;
    int need_len = 0;
    std::optional<std::string> conlen = client->get_val("CONTENT_LENGTH");
    if (conlen.has_value()){
      need_len = std::stoi(conlen.value());
    }else{
      return 400;
    }
    
    char* buf = new char[need_len+1];

    std::cout<<"need_len: "<<need_len<<std::endl;
    /**
     * 此处需要处理buf不足的异常 
     */
    size_t rev_length = client->recv_body(buf,need_len);
    buf[need_len] = '\0';
    std::cout<<buf<<std::endl;
    client->send_status(200,"OK");
    client->send_header("Content-Length","0");
    client->complete_header();
    client->complete_request();
    delete[] buf;
    return 200;

}

void process(verb method,ClientIO* client, boost::asio::yield_context yield,int& http_ret){
  int ret = 0;
  if (method == verb::put || method == verb::post){
    ret = putobj(client,yield);
  }else if(method == verb::get){
    return;
  }else if(method == verb::head){
    return;
  }else{
    http_ret = 400;
  }
  http_ret = ret;
}