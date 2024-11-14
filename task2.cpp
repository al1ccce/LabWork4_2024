#include <thread>
#include <iostream>
#include <vector>
#include <random>
#include <mutex>

using namespace std;

struct tovar{
    string code;
    int amount;
    int price;
};

vector<tovar> generateTovars(int size) {
    vector<tovar> tovars(size);
    random_device rd; 
    knuth_b gen(rd());
    uniform_int_distribution<> codeDist(100000, 999999);
    uniform_int_distribution<> amountDist(1, 10);
    uniform_real_distribution<> priceDist(1, 100);

    for (int i = 0; i < size; ++i) {
        tovars[i].code = to_string(codeDist(gen));
        tovars[i].amount = amountDist(gen);
        tovars[i].price = priceDist(gen);
    }

    return tovars;
}

bool matchPattern(const string& code, const string& pattern) {
    if (code.size() != pattern.size()) return false;
    for (size_t i = 0; i < code.size(); ++i) {
        if (pattern[i] != '?' && pattern[i] != code[i]) {
            return false;
        }
    }
    cout << code << endl;
    return true;
}

int processTovars(const vector<tovar>& tovars, const string& pattern) {
    int totalCost = 0;
    for (const auto& tovar : tovars) {
        if (matchPattern(tovar.code, pattern)) {
            totalCost += tovar.amount * tovar.price;
        }
    }
    cout << totalCost << endl;
    return totalCost;
}

mutex mtx;

void processTovarsThread(const vector<tovar>& tovars, const string& pattern, int& totalCost, int start, int end) {
    int localTotalCost = 0;
    for (int i = start; i < end; ++i) {
        if (matchPattern(tovars[i].code, pattern)) {
            localTotalCost += tovars[i].amount * tovars[i].price;
        }
    }
    lock_guard<mutex> lock(mtx);
    totalCost += localTotalCost;
}

int processTovarsParallel(const vector<tovar>& tovars, const string& pattern, int numThreads) {
    int size = tovars.size();
    int chunkSize = size / numThreads; // Сколько обрабатывает каждый поток
    vector<thread> threads;
    int totalCost = 0;

    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? size : start + chunkSize;
        threads.emplace_back(processTovarsThread, ref(tovars), ref(pattern), ref(totalCost), start, end);
    }

    for (auto& t : threads) {
        t.join();
    }

    return totalCost;
}

int main() {
    int size = 100;
    int numThreads = 4;
    string pattern = "?123??";

    vector<tovar> tovars = generateTovars(size);

    // Последовательная обработка
    auto start = chrono::high_resolution_clock::now();
    int singleCost = processTovars(tovars, pattern);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> singleTime = end - start;

    // Многопоточная обработка
    start = chrono::high_resolution_clock::now();
    int multiCost = processTovarsParallel(tovars, pattern, numThreads);
    end = chrono::high_resolution_clock::now();
    chrono::duration<double> multiTime = end - start;

    // Вывод результатов
    cout << "Обработка в однопоточном режиме: " << singleTime.count() << " секунд\n";
    cout << "Обработка в многопоточном режиме: " << multiTime.count() << " секунд\n";
    cout << "Итоговая стоимость (однопоточность): " << singleCost << "\n";
    cout << "Итоговая стоимость (многопоточность): " << multiCost << "\n";

    return 0;
}
