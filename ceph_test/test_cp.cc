#include <iostream>
#include <mutex>
#include <thread>
#include <memory>
#include <vector>
#include <string>

class context{

public:
	virtual int run()=0;
};

class TaskManager{
private:
	std::mutex mtx;
	std::condition_variable cond;

	std::thread m_handle;

	std::thread::id thread_id;
	bool stopped;
	std::string thread_name;

private:
	virtual void entry();
	std::vector<std::shared_ptr<context>> task;

    void _process(std::vector<std::shared_ptr<context>>&);

public:
	void create(const std::string& name){
		thread_name = name;
		m_handle = std::thread(&TaskManager::entry,this);
		thread_id = m_handle.get_id();
	}

	void join(){
		if (m_handle.joinable())
			m_handle.join();
	}
  	void detach(){
  		m_handle.detach();
  	}
	int add_task(std::shared_ptr<context>);
	void stop(){
        if (stopped)
            return;
		std::lock_guard<std::mutex> lock{mtx};
		stopped = true;
		cond.notify_all();
	}
};

void TaskManager::entry(){
    std::unique_lock<std::mutex> lock{mtx};
	while (!stopped || !task.empty()){
        cond.wait(lock,[this](){return (!task.empty() || stopped);});
        std::vector<std::shared_ptr<context>> tmp;
        tmp.swap(task);
        lock.unlock();
        _process(tmp);
    }
}

void TaskManager::_process(std::vector<std::shared_ptr<context>>& tsk){
    for (const auto& tk:tsk){
        tk->run();
    }
}

int TaskManager::add_task(std::shared_ptr<context> ts){
    if (stopped)
        return -1;
    {
        std::lock_guard<std::mutex> lock{mtx};
        task.push_back(ts);
        cond.notify_all();
    }
    return 0;
}


class test: public context{
private:
    std::string name;
public:
    test(const std::string& name):name(name){}
    int run() override {
        std::cout<<"Task Run: "<<name<<std::endl;
        return 0;
    }
 };

int main(int argc,char* argv[]){
    std::shared_ptr<TaskManager> tm = std::make_shared<TaskManager>();
    tm->create("TaskManager");
    tm->add_task(std::make_shared<test>("TaskA"));
    tm->add_task(std::make_shared<test>("TaskB"));
    tm->add_task(std::make_shared<test>(std::string("TaskC")));
    tm->stop();
    tm->join();
}