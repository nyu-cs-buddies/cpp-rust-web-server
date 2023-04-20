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
