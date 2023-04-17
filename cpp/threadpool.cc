#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>

static unsigned int exec_count = 0;
static std::mutex exec_mutex;

class ThreadPool {
 private:
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop_flag;

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
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop_flag = true;
    }
    condition.notify_all();
    for (auto& thread : threads) {
      thread.join();
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
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop_flag = true;
    }
    condition.notify_all();
    for (auto& thread : threads) {
      thread.join();
    }
    // std::condition_variable cv;
    // std::unique_lock<std::mutex> lock(busy_thread_count_mutex);
    // cv.wait(lock, [this] {
    //     return (this->busy_thread_count == 0) && this->tasks.empty();
    // });

  }

  auto size() {
    return tasks.size();
  }
};



void hello_world() {
  {
    std::unique_lock<std::mutex> lock(exec_mutex);
    exec_count++;
  }
  std::cout << "Hello world from " << std::this_thread::get_id() << std::endl;
}

int main() {
  ThreadPool pool(4);
  pool.add_task([]() {
    std::cout << "Hello, world!" << std::endl;
  });
  pool.add_task(hello_world);
  pool.add_task(hello_world);
  pool.add_task(hello_world);
  pool.add_task(hello_world);
  pool.add_task(hello_world);
  pool.add_task(hello_world);
  pool.add_task(hello_world);
  pool.add_task(hello_world);
  pool.add_task(hello_world);
  pool.add_task(hello_world);
  //pool.stop();
  sleep(2);
  {
    std::unique_lock<std::mutex> lock(exec_mutex);
    std::cout << "exec_count for hello_world() = " << exec_count << std::endl;
  }
  return 0;
}
