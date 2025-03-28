
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <chrono>
#include <condition_variable>
#include <optional>

class ConcurrentQueue {
public:
    ConcurrentQueue(size_t capacity)
        : capacity(capacity) {}

    void push(int value) {
        std::unique_lock<std::mutex> lock(mutex);
        // Wait until there is space in the queue
        not_full.wait(lock, [this] { return queue.size() < capacity; });
        queue.push(value);
        not_empty.notify_one(); // Notify that an item is available
    }

    std::optional<int> pop() {
        std::unique_lock<std::mutex> lock(mutex);
        // Wait until there is at least one item in the queue
        not_empty.wait(lock, [this] { return !queue.empty(); });
        int value = queue.front();
        queue.pop();
        not_full.notify_one(); // Notify that there is space in the queue
        return value;
    }

    bool is_empty() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

private:
    std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;
    std::queue<int> queue;
    size_t capacity;
};

class AtomicCounter {
public:
    AtomicCounter() : count(0) {}

    void increment() {
        count.fetch_add(1, std::memory_order_relaxed);
    }

    int get() const {
        return count.load(std::memory_order_relaxed);
    }

private:
    std::atomic<int> count;
};

void producer(ConcurrentQueue& queue, AtomicCounter& counter, int id) {
    for (int i = 0; i < 10; ++i) {
        int value = id * 100 + i;
        queue.push(value);
        counter.increment();
        std::cout << "Producer " << id << " produced: " << value << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer(ConcurrentQueue& queue, AtomicCounter& counter, int num_producers) {
    int consumed_items = 0;
    while (consumed_items < num_producers * 10) { // Stop after consuming all produced items
        auto value = queue.pop();
        if (value) {
            std::cout << "Consumer consumed: " << *value << "\n";
            consumed_items++;
        }
    }
}

int main() {
    ConcurrentQueue queue(5);
    AtomicCounter counter;

    std::vector<std::thread> producers;
    for (int i = 0; i < 3; ++i) {
        producers.emplace_back(producer, std::ref(queue), std::ref(counter), i);
    }

    std::thread consumer_thread(consumer, std::ref(queue), std::ref(counter), 3); // Pass number of producers

    for (auto& producer : producers) {
        producer.join();
    }

    consumer_thread.join();

    std::cout << "Total items produced: " << counter.get() << "\n";

    return 0;
}
