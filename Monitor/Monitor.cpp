#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
int value;

void producer()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);
        value = rand() % 10 + 1;
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
        cv.wait(lock);
        std::cout << "        consumed " << value << '\n';
        lock.unlock();
    }
}

int main() {
    std::thread producerThread(producer);
    std::thread consumerThread(consumer);
    producerThread.join();
    consumerThread.join();
}
