#pragma once



#include<sys/sysinfo.h>
#include<thread>
#include<vector>
#include<queue>
#include<functional>
#include<mutex>
#include<atomic>
#include<condition_variable>

class ListDatabase;
class Balancer{
    int num_threads;
    bool terminate_db=false;
    std::vector<std::thread> threads;
    std::mutex job_lock;
    std::condition_variable mutex_condition;
    std::queue<std::function<void(int, ListDatabase*)> >jobs;
    void thread_loop();
public:
    Balancer();
    void queue_job(std::function<void(int, ListDatabase*)>);
    void stop();
};