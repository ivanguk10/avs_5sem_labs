#include <iostream>
#include <iostream>
#include <queue>
#include <thread>
#include <atomic>
#include <string>
#include <mutex>
#include <condition_variable>

int TaskNum = 4 * 1024 * 1024;
const int ProducerNum[] = { 1, 2, 4 };
const int ConsumerNum[] = { 1, 2, 4 };
const int QueueSize[] = { 1, 4, 16 };
int amount = 0;
int producerAmount;
int expectedAmount = 0;
std::mutex sum_mutex;

template <typename T>
class queue {
public:
    virtual void push(T val) = 0;
    virtual bool pop(T& val) = 0;
};
template <typename T>
class dynamic_queue : public queue<T> {
    std::queue<T> _q;
    std::mutex mutex;
public:
    void push(T val) override {
        std::unique_lock<std::mutex> uniqueLock(mutex);
        _q.push(val);
    };

    bool pop(T& val) override {
        if (amount == expectedAmount) {
            return false;
        }
        std::unique_lock<std::mutex> uniqueLock(mutex);
        if (_q.empty()) {
            uniqueLock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            uniqueLock.lock();
        }
        if (_q.empty()) {
            return false;
        }
        val = _q.front();
        _q.pop();
        return true;
    };
};
template <typename T>
class fixed_size_mutex_queue : public queue<T> {
private:
    std::queue<T> _q;
    int _size;
    std::mutex mutex;
    std::condition_variable conditionVariable;
public:
    void push(T val) override {
        std::unique_lock<std::mutex> uniqueLock(mutex);
        if (is_full()) {
            conditionVariable.wait(uniqueLock, [&]() { return !is_full(); });
        }
        _q.push(val);
        conditionVariable.notify_all();
    };

    bool pop(T& val) override {
        if (amount == expectedAmount) {
            return false;
        }
        std::unique_lock<std::mutex> uniqueLock(mutex);

        if (_q.empty()) {
            uniqueLock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            uniqueLock.lock();
        }

        if (_q.empty()) {
            return false;
        }
        val = _q.front();
        _q.pop();
        conditionVariable.notify_all();
        return true;
    };

    bool is_full() {
        return _q.size() == _size;
    }

    explicit fixed_size_mutex_queue(std::size_t size) {
        _size = size;
    }

    fixed_size_mutex_queue() {
        _size = 1;
    };
};

void consume(queue<uint8_t>* _q) {
    int innerAmount = 0;
    uint8_t val = 0;
    while(producerAmount != 0){
        while (_q->pop(val)) {
            innerAmount += 1;
        }
    }
    std::unique_lock<std::mutex> uniqueLock(sum_mutex);
    amount += innerAmount;
}

void produce(queue<uint8_t>* _q) {
    for (int i = 0; i < TaskNum; i++) {
        _q->push(1);
    }
    sum_mutex.lock();
    producerAmount -= 1;
    sum_mutex.unlock();
}
template<typename T>
void start(int producerNum,int consumerNum,queue<T>* _q)
{
    std::vector<std::thread> producerThreads(producerNum);
    std::vector<std::thread> consumerThreads(consumerNum);
    expectedAmount = producerNum * TaskNum;
    std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < consumerNum; i++) {
        consumerThreads[i] = std::thread(consume, _q);
    }
    for (int i = 0; i < producerNum; i++) {
        producerThreads[i] = std::thread(produce, _q);
    }

    for (auto& thread : consumerThreads) {
        thread.join();
    }
    for (auto& thread : producerThreads) {
        thread.join();
    }
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    double time_interval = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout<< "Expected amount:\t" << producerNum * TaskNum << std::endl;
    std::cout<< "Actual amount:\t\t" << amount << std::endl;
    std::cout<< "Time of work:\t"<< long(time_interval)<< "ms"<< std::endl << std::endl;
}
void Task2() {
    std::cout<< "Task 2\nDynamicQueue\n\n";
    dynamic_queue<uint8_t> dynamic_q;
    for (int producers : ProducerNum) {
        for (int consumers : ConsumerNum) {
            amount = 0;
            producerAmount = producers;
            std::cout<<"[DynamicQueue, ProducerNum: " + std::to_string(producers) + ", ConsumerNum: " +
                       std::to_string(consumers) + "]\n";
            start(producers, consumers, &dynamic_q);
        }
    }
    std::cout<< "Task 2\nFixedSizeMutexQueue\n\n";
    TaskNum = 1024;
    std::cout<< "TaskNum changed to "<< TaskNum<< std::endl<< std::endl;
    for (int size : QueueSize) {
        fixed_size_mutex_queue<uint8_t> fixed_size_mutex_q(size);
        for (int producers : ProducerNum) {
            for (int consumers : ConsumerNum) {
                amount = 0;
                producerAmount = producers;
                std::cout<<"[FixedSizeMutexQueue, Queue Size: " + std::to_string(size) +
                           ", ProducerNum: " + std::to_string(producers) + ", ConsumerNum: " +
                           std::to_string(consumers) +
                           "]\n";
                start(producers, consumers, &fixed_size_mutex_q);
            }
        }
    }
}



