#pragma once

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

  void fetch_task();

 public:
  ThreadPool() = delete;
  ThreadPool(unsigned int num_threads);
  ~ThreadPool();

  auto size() { return tasks.size(); }
  void add_task(const std::function<void()>& task);
  void stop();
};
