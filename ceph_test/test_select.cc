#include <iostream>
#include <mutex>
#include <thread>
#include <memory>
#include <map>
#include <string>
#include <functional>
#include <array>
#include <string_view>
#include <algorithm>

#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

int pipe_ocloexc(int pipefd[2]){
     /*
     ** pipe init
     */
    if (pipe2(pipefd, O_CLOEXEC) < 0){
        return -1;
    }   
    if (::fcntl(pipefd[0], F_SETFL, O_NONBLOCK) < 0){
         ::close(pipefd[1]);
         ::close(pipefd[0]);
         return -1;
    }
    return 0;
}

struct io_handler{
    io_handler(const std::string user):user(user){
        int r = pipe_ocloexc(pipefd);
        if (r < 0){
            exit(-1);
        }
    }
    std::string user;
    int pipefd[2];
    std::function<int(void*)> callback;
};

int read(int fd){
    std::array<char, 256> buf;
    int ret = ::read(fd, (void *)buf.data(), buf.size());
    if (ret < 0) { 
        return -1;
    }
    std::cout<<"Read Data: "<<std::string_view(buf.data(),ret)<<std::endl;
    return 0;
}

int process(void* arg){
    io_handler* ptr = (io_handler*)arg;
    //std::cout<<"IO: "<<ptr->user<<std::endl;
	int ret = read(ptr->pipefd[0]);
	if (ret < 0 )
		return -1;
    return 0;
}

class TaskManager{
private:
	std::mutex mtx;
    int thread_pipe[2];

	std::thread m_handle;

	std::thread::id thread_id;
	std::string thread_name;

    bool stopped;

private:
	virtual void entry();
    using mio = std::map<int,std::shared_ptr<io_handler>>;
	mio io_callback;

    int signal_thread();
    int clear_signal(int);

public:
	int create(const std::string& name);

	void join();
  	void detach();
	int register_fd(std::shared_ptr<io_handler>);
	void unregister_fd(std::shared_ptr<io_handler>);
	void stop();
	virtual ~TaskManager(){
	    ::close(thread_pipe[1]);
        ::close(thread_pipe[0]);
	}
};

int TaskManager::create(const std::string& name){
	thread_name = name;

    if (pipe_ocloexc(thread_pipe)<0)
        return -1;

	m_handle = std::thread(&TaskManager::entry,this);
	thread_id = m_handle.get_id();
    return 0;
}

int TaskManager::clear_signal(int fd)
{
    std::array<char, 256> buf;
    int ret = ::read(fd, (void *)buf.data(), buf.size());
    if (ret < 0) { 
        return -1;
    }
    return 0;
}

void TaskManager::join(){
	if (m_handle.joinable())
		m_handle.join();
}

void TaskManager::detach(){
	m_handle.detach();
}

void TaskManager::stop(){
    if (stopped)
        return;
    stopped = true;
	signal_thread();
}

void TaskManager::entry(){
	fd_set fdread;
	fd_set fdwrite;
	fd_set fdexcep;
	while (!stopped || !io_callback.empty()){
		int maxfd = -1;
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		FD_SET(thread_pipe[0],&fdread);
		maxfd = thread_pipe[0]+1;

		std::unique_lock<std::mutex> lock{mtx};
		for (const auto& io:io_callback){
			FD_SET(io.first,&fdread);
			if (io.first >= maxfd){
				maxfd = io.first+1;
			}
		}
		lock.unlock();
		int ret = select(maxfd,&fdread,&fdwrite,&fdexcep,NULL);
		if ( ret < 0)
			return;
		lock.lock();
		for (const auto& io:io_callback){
			if (FD_ISSET(io.first,&fdread)){
				//clear_signal(io.first);
				(io.second)->callback(io.second.get());
			}
		}
		if (FD_ISSET(thread_pipe[0],&fdread)){
			clear_signal(thread_pipe[0]);
		}
		lock.unlock();
    }
}

int TaskManager::signal_thread(){
    uint32_t buf = 0;
    int ret = write(thread_pipe[1], (void *)&buf, sizeof(buf));
    if (ret < 0) {
        return -1;
    }
    return 0;
}

int TaskManager::register_fd(std::shared_ptr<io_handler> handler){
    if (stopped)
        return -1;
    {
        std::lock_guard<std::mutex> lock{mtx};
        io_callback.insert(std::pair<int,std::shared_ptr<io_handler>>(handler->pipefd[0],handler));
        signal_thread();
    }
    return 0;
}

void TaskManager::unregister_fd(std::shared_ptr<io_handler> io){
	{
		std::lock_guard<std::mutex> lock{mtx};
		io_callback.erase(io->pipefd[0]);
		signal_thread();
	}
}

int test(std::shared_ptr<io_handler> io){
    std::array<char,256> buf;
    std::copy(io->user.cbegin(),io->user.cend(),buf.data());
    int ret = write(io->pipefd[1], (void *)&buf, io->user.size());
    if (ret < 0) {
        return -1;
    }
    return 0;
}

int main(int argc,char* argv[]){
    std::shared_ptr<TaskManager> tm = std::make_shared<TaskManager>();
    tm->create("TaskManager");
    std::shared_ptr<io_handler> io1 = std::make_shared<io_handler>("io_1");
    io1->callback = process;
    std::shared_ptr<io_handler> io2 = std::make_shared<io_handler>("io_2");
    io2->callback = process;
    std::shared_ptr<io_handler> io3 = std::make_shared<io_handler>("io_3");
    io3->callback = process;
    tm->register_fd(io1);
    tm->register_fd(io2);
    tm->register_fd(io3);

    test(io1);
    test(io3);
    test(io2);

	sleep(3);
	
	tm->unregister_fd(io1);
	tm->unregister_fd(io2);
	tm->unregister_fd(io3);
    tm->stop();
    tm->join();
}
