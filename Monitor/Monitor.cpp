#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv; // Уведомления между потоками
int value; // Общая переменная, которой обмениваются потоки
bool dataReady = false; // Проверка готовности

void producer()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);
        value = rand() % 10 + 1;
        dataReady = true;
        std::cout << "produced " << value << '\n';
        cv.notify_one();
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void consumer()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] {return dataReady; });
        std::cout << "        consumed " << value << '\n';
        dataReady = false;
        lock.unlock();
    }
}

int main() {
    std::thread producerThread(producer); // Создание потока производителя
    std::thread consumerThread(consumer); // Создание потока потребителя
    producerThread.join();
}
