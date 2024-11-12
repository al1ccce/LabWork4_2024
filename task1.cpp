#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <condition_variable>
#include <atomic>
#include <semaphore> 
#include <barrier>   

using namespace std;

mutex mtx; 
int num_th = 100;

class SlimSemaphore {
public:
    SlimSemaphore(int count = 0) : count(count) {}

    void acquire() {
        unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; }); // ждем, пока значение count не станет больше 0
        --count;
    }

    void release() {
        unique_lock<mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }

private:
    mutex mtx;
    condition_variable cv;
    int count;
};

void t_mutex() {
    static mutex mtx; 
    mtx.lock();
    cout << char(33 + rand() % 90) << ' ';
    mtx.unlock();
}

void t_semaphore() {
    static counting_semaphore<1> sem(1); // Семафор доступен для захвата
    sem.acquire(); // Захват семафора (если = 0 - блокировка, пока другой поток не инкрементирует счетчик)
    cout << char(33 + rand() % 90) << ' ';
    sem.release(); // Увеличение семафора на 1, разблокировка
}

void t_slim_semaphore() {
    static SlimSemaphore sem(1); // Инициализация семафора с начальным значением 1
    sem.acquire(); // Захват семафора (если = 0 - блокировка)
    std::cout << char(33 + rand() % 90) << ' ';
    sem.release(); // Увеличение семафора на 1, разблокировка
}

void t_spinlock() {
    static atomic<bool> spinLock(false); // Спинлок изначально свободен 
    while (spinLock.exchange(true)) {} // Поток проверяет, свободен ли спинлок
    cout << char(33 + rand() % 90) << ' '; // После захвата спинлока выполняется критический блок
    spinLock.store(false); // Освобождение спинлока
}

void t_spinwait() {
    static atomic<bool> spinWait(false);
    while (spinWait.exchange(true)) {
        this_thread::yield(); // Готов отдать процессорное время другим потокам
    }
    cout << char(33 + rand() % 90) << ' ';
    spinWait.store(false);
}

void t_barrier() {
    static barrier Barrier(num_th); // Барьер на определенное количество потоков
    mtx.lock();
    cout << char(33 + rand() % 90) << ' ';
    mtx.unlock();
    Barrier.arrive_and_wait(); // Блокирует поток до тех пор, пока все потоки не достигнут барьера.
}

void t_monitor() {
    static condition_variable cv;
    static bool ready = false;

    unique_lock<mutex> lock(mtx); 
    cv.wait(lock, []{ return ready; }); // Поток ждет, пока флаг ready не станет true.
    cout << char(33 + rand() % 90) << ' ';
    ready = false; // Сбрасываем условие ready в false, чтобы другие потоки могли проверить условие и продолжить выполнение
    cv.notify_all(); // Уведомить все ожидающие потоки
}

void runThreads(void (*threadFunction)(), const string& PrimoName) {
    vector<thread> threads;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < num_th; ++i) {
        threads.emplace_back(threadFunction);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> restime = end - start;

    cout << "\nВремя для " << PrimoName << ": " << restime.count() << " секунд\n";
}

int main() {
    srand(time(0)); 

    runThreads(t_mutex, "Mutex");
    runThreads(t_semaphore, "Semaphore");
    runThreads(t_slim_semaphore, "Slim semaphore");
    runThreads(t_spinlock, "SpinLock");
    runThreads(t_spinwait, "SpinWait");
    runThreads(t_barrier, "Barrier");
    runThreads(t_monitor, "Monitor");

    return 0;
}
