//
// Created by ivanguk10 on 07.12.2020.
//
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <mutex>
#include <functional>
#include <algorithm>

const int NumTasks = 1024 * 1024;

std::mutex mutex;
int mutexInd = 0;

std::atomic<int> atomicInd(0);

void Increment(std::vector<unsigned char> &vector, int sleep, bool isAtomic) {
    std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
    int ind;
    while (true){
        if (isAtomic) {
            ind = atomicInd.fetch_add(1);
        }
        else {
            mutex.lock();
            ind = mutexInd++;
            mutex.unlock();
        }
        if (sleep) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(sleep));
        }
        if (ind >= NumTasks) {
            break;
        }
        vector.at(ind)++;
    }
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    double time_interval = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << long(time_interval) << "ms" << std::endl;
}

void Start(int sleep, int NumThreads, bool isAtomic) {
    std::thread threads[NumThreads];
    std::vector<unsigned char> vector(NumTasks, 0);
    if(isAtomic) {
        std::cout << "Atomic counter with " << NumThreads << " threads and " << sleep << " nanoseconds sleep\n";
    }
    else{
        std::cout<<"Mutex counter with " << NumThreads << " threads and " << sleep << " nanoseconds sleep\n";
    }

    for(int i = 0; i < NumTasks; i++){
        vector[i] = 0;
    }

    if(isAtomic) {
        atomicInd.store(0);
    }
    else{
        mutex.lock();
        mutexInd = 0;
        mutex.unlock();
    }

    for (int i = 0; i < NumThreads; i++) {
        threads[i] = std::thread(Increment, std::ref(vector), sleep, isAtomic);
    }
    for (int i = 0; i < NumThreads; i++) {
        threads[i].join();
    }

    bool check = std::all_of(vector.begin(), vector.end(), [](int i) { return i == 1; });
    if(check){
        std::cout << "Array is correct\n\n";
    }
    else{
        std::cout << "Incorrect array\n\n";
    }
}

void Task1() {
    int NumThreads[] = {4, 8, 16, 32};
    for (int i : NumThreads) {
        Start(0, i, true);
        Start(0, i, false);
        Start(10, i, true);
        Start(10, i, false);
    }

}



