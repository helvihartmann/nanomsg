#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <string>
#include <cassert>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

class Shmmanager {
    const size_t size_ = 1000000;
    std::string shared_memory_identifier_ = "HH";
    boost::interprocess::shared_memory_object *shm_ = nullptr;
    boost::interprocess::mapped_region *region_ = nullptr;
    
public:
    void connect_write();
    void connect_read();
    void disconnect();
    void fill();
    void xor_all();
    void remove();

};



