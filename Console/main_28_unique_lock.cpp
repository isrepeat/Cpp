//#include <Windows.h>
//#include <condition_variable>
//#include <functional>
//#include <iostream>
//#include <vector>
//#include <thread>
//#include <mutex>
//#include <queue>
//
//
//
//class ConcurrentQueue {
//public:
//    ConcurrentQueue() = default;
//    ~ConcurrentQueue() = default;
//
//    int Pop() {
//        std::unique_lock<std::mutex> lock(mx);
//        Sleep(1);
//        Sleep(1);
//        cv.wait(lock, [&] { 
//            return !this->items.empty(); // if return false - do wait and unlock mx inner
//            });
//        auto package = std::move(items.front());
//        items.pop();
//        cv.notify_one();
//        return package;
//    }
//
//    void Push(int item) {
//        std::unique_lock<std::mutex> lock(mx);
//        Sleep(1);
//        Sleep(1);
//        //cv.wait(lock);
//        items.push(item);
//        cv.notify_one();
//    }
//
//private:
//    std::mutex mx;
//    std::condition_variable cv;
//    std::queue<int> items;
//};
//
//int main() {
//    ConcurrentQueue concurrentQueue;
//
//    auto thPop = std::thread([&concurrentQueue] {
//        Sleep(100);
//        auto item = concurrentQueue.Pop();
//        Sleep(1);
//        Sleep(1);
//        });
//
//    auto thPush = std::thread([&concurrentQueue] {
//        Sleep(1000);
//        concurrentQueue.Push(17);
//        Sleep(1);
//        Sleep(1);
//        });
//
//
//
//    while (true) {
//        Sleep(1);
//    };
//
//    if (thPop.joinable())
//        thPop.join();
//    
//    if (thPush.joinable())
//        thPush.join();
//
//    return 0;
//}