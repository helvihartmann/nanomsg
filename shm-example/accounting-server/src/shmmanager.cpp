#include "shmmanager.h"

void Shmmanager::connect_write() {
    assert(!shm_);
    assert(!region_);
    
    shm_ = new boost::interprocess::shared_memory_object(boost::interprocess::open_or_create,
                                                         shared_memory_identifier_.c_str(),
                                                         boost::interprocess::read_write);
    
    shm_->truncate(size_);
    
    region_ = new boost::interprocess::mapped_region(*shm_, boost::interprocess::read_write);
}

void Shmmanager::connect_read() {
    assert(!shm_);
    assert(!region_);
    
    shm_ = new boost::interprocess::shared_memory_object(boost::interprocess::open_only,
                                                         shared_memory_identifier_.c_str(),
                                                         boost::interprocess::read_only);
    
    region_ = new boost::interprocess::mapped_region(*shm_, boost::interprocess::read_only);
}

void Shmmanager::disconnect() {
    delete region_;
    delete shm_;
}

void Shmmanager::fill() {
    std::random_device random_device;
    std::uniform_int_distribution<unsigned char> distribution;
    unsigned char *buf = reinterpret_cast<unsigned char*>(region_->get_address());
    for (unsigned int i = 0; i < region_->get_size(); ++i) {
        buf[i] = distribution(random_device);
    }
}

void Shmmanager::xor_all() {
    unsigned char result = 0;
    unsigned char *buf = reinterpret_cast<unsigned char*>(region_->get_address());
    for (unsigned int i = 0; i < region_->get_size(); ++i) {
        result ^= buf[i];
    }
    std::cout << "xor_all: " << static_cast<unsigned int>(result) << std::endl;
}

void Shmmanager::remove() {
    boost::interprocess::shared_memory_object::remove(shared_memory_identifier_.c_str());
}