#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>

class ThreadPool {
 private:
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop_flag;
  bool stopped;

  void fetch_task() {
    while (true) {
      std::function<void()> task;
      {
        std::unique_lock<std::mutex> lock(queue_mutex);
        condition.wait(lock, [this] {
            return this->stop_flag || !this->tasks.empty();
        });
        if (stop_flag) {
          return;
        }
        task = std::move(tasks.front());
        tasks.pop();
      }
      task();
    }
  }

 public:
  ThreadPool() = delete;
  ThreadPool(unsigned int num_threads) : stop_flag(false) {
    threads.resize(num_threads);
    for (auto& thread : threads) {
      thread = std::move(std::thread(&ThreadPool::fetch_task, this));
    }
  }

  ~ThreadPool() {
    if (! stopped) {
      stop();
    }
  }

  void add_task(const std::function<void()>& task) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      tasks.push(task);
    }
    condition.notify_one();
  };

  void stop() {
     // std::condition_variable cv;
    // std::unique_lock<std::mutex> lock(busy_thread_count_mutex);
    // cv.wait(lock, [this] {
    //     return (this->busy_thread_count == 0) && this->tasks.empty();
    // });
    
    // TODO(allenpthuang): naive workaround; might need to use condition variable.
    while (tasks.size() > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop_flag = true;
    }
    condition.notify_all();
    for (auto& thread : threads) {
      thread.join();
    }
    stopped = true;
  }

  auto size() {
    return tasks.size();
  }
};
