#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <condition_variable>

using namespace std;

mutex mtx;

condition_variable cv;
int numReaders = 0;
bool writerActive = false;

void readData() {
    
    unique_lock<mutex> lock(mtx);
    // Ждем, пока нет активных писателей
    cv.wait(lock, [] { return !writerActive; });

    numReaders++;
    lock.unlock();

    // Критическая секция для чтения
    cout << " Reading data..." << endl;

    lock.lock();
    numReaders--;
    if (numReaders == 0) {
        cv.notify_all(); // Уведомляем писателей, если больше нет читателей
    }
    lock.unlock();
}

void writeData() {
    unique_lock<mutex> lock(mtx);
    // Ждем, пока нет активных читателей и писателей
    cv.wait(lock, [] { return numReaders == 0 && !writerActive; });
    
    writerActive = true;
    lock.unlock();

    // Критическая секция для записи
    cout << "Writing data... numReaders = " << numReaders << endl;
    this_thread::sleep_for(chrono::milliseconds(100)); // Симуляция записи

    lock.lock();
    writerActive = false;
    cv.notify_all(); // Уведомляем всех, что писатель завершил
    lock.unlock();
}

int main() {
    vector<thread> threads;
    int numThreads = 50;

    for (int i = 0; i < numThreads; ++i) {
        if (i % 2 == 0) {
            threads.emplace_back(readData);
        } else {
            threads.emplace_back(writeData);
        }
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
