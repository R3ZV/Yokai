#include "include/balancer.h"
#include<sys/sysinfo.h>
#include<thread>
#include "include/list_database.h"
Balancer::Balancer(){
    num_threads=8;
    for(int i=0; i<num_threads; i++){
        threads.emplace_back(std::thread(&Balancer::thread_loop,this) );
    }
}
auto Balancer::thread_loop() -> void{
    while(true){
       std::function<void(int, ListDatabase*)> job;
       std::unique_lock<std::mutex> lock(job_lock);
       mutex_condition.wait(lock, [this] {
                return !jobs.empty() || this->terminate_db;
       });
       if (terminate_db) {
            return;
       }
       job = jobs.front();
       jobs.pop();
    }
}

auto Balancer::queue_job( std::function<void(int, ListDatabase*)> job ) -> void{
    std::unique_lock<std::mutex> lock(job_lock); //so multiple instances won't access this
    jobs.push(job);
}

auto Balancer::stop() -> void{
    std::unique_lock<std::mutex> lock(job_lock);
    terminate_db = true;
    mutex_condition.notify_all();
    for (std::thread& active_thread : threads) {
        active_thread.join();
    }
    threads.clear();
}