#include "threadpool.h"

// static unsigned int exec_count = 0;
// static std::mutex exec_mutex;

void ThreadPool::fetch_task() {
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

ThreadPool::ThreadPool(unsigned int num_threads) : stop_flag(false) {
  threads.resize(num_threads);
  for (auto& thread : threads) {
    thread = std::move(std::thread(&ThreadPool::fetch_task, this));
  }
}

ThreadPool::~ThreadPool() {
  if (! stopped) {
    stop();
  }
}

void ThreadPool::add_task(const std::function<void()>& task) {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.push(task);
  }
  condition.notify_one();
};

void ThreadPool::stop() {
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



void hello_world() {
  // {
  //   std::unique_lock<std::mutex> lock(exec_mutex);
  //   exec_count++;
  // }
  std::cout << "Hello world from " << std::this_thread::get_id() << std::endl;
}

void test() {
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
  pool.stop();
  // sleep(1);
  // {
  //   std::unique_lock<std::mutex> lock(exec_mutex);
  //   std::cout << "exec_count for hello_world() = " << exec_count << std::endl;
  // }
}
