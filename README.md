
## Documentation

### Atomic-Counter

The provided C++ code implements a simple producer-consumer pattern using a thread-safe queue and atomic counter. Let's break down the code step by step:

### 1. **Includes and Libraries**
```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <chrono>
#include <condition_variable>
#include <optional>
```
- **iostream** for input and output operations.
- **thread** for creating and managing threads.
- **atomic** for atomic operations on shared data.
- **vector** for using dynamic arrays.
- **queue** for using queue data structures.
- **chrono** for time-related functions.
- **condition_variable** for thread synchronization.
- **optional** for returning a value that may or may not be present.

### 2. **ConcurrentQueue Class**
This class implements a thread-safe queue with a fixed capacity using mutexes and condition variables.

#### Members:
- `std::mutex mutex`: A mutex for protecting access to the queue.
- `std::condition_variable not_empty`: A condition variable to signal when the queue is not empty.
- `std::condition_variable not_full`: A condition variable to signal when the queue is not full.
- `std::queue<int> queue`: The actual queue to hold integer values.
- `size_t capacity`: The maximum number of items the queue can hold.

#### Methods:
- **push(int value)**: 
  - Acquires a lock on the mutex.
  - Waits until there's space in the queue.
  - Pushes the value into the queue and notifies one waiting thread that the queue is not empty.
  
- **pop()**: 
  - Acquires a lock on the mutex.
  - Waits until the queue is not empty.
  - Pops a value from the queue and notifies one waiting thread that the queue is not full. Returns the value as an optional.

- **is_empty()**: 
  - Acquires a lock and checks if the queue is empty.

### 3. **AtomicCounter Class**
This class provides a thread-safe counter using atomic operations.

#### Members:
- `std::atomic<int> count`: An atomic integer to store the count.

#### Methods:
- **increment()**: Atomically increments the count.
- **get()**: Returns the current count.

### 4. **Producer Function**
```cpp
void producer(ConcurrentQueue& queue, AtomicCounter& counter, int id) {
    for (int i = 0; i < 10; ++i) {
        int value = id * 100 + i; // Generate a unique value for each item
        queue.push(value); // Push it to the concurrent queue
        counter.increment(); // Increment the counter
        std::cout << "Producer " << id << " produced: " << value << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
    }
}
```
- Each producer generates and pushes 10 values into the queue, each identified uniquely by its `id`.
- It also increments the atomic counter and simulates work with a sleep.

### 5. **Consumer Function**
```cpp
void consumer(ConcurrentQueue& queue, AtomicCounter& counter, int num_producers) {
    int consumed_items = 0;
    while (consumed_items < num_producers * 10) { // Consume until all produced items are consumed
        auto value = queue.pop();
        if (value) {
            std::cout << "Consumer consumed: " << *value << "\n";
            consumed_items++;
        }
    }
}
```
- The consumer pops values from the queue until it has consumed all items produced by the producers.

### 6. **Main Function**
```cpp
int main() {
    ConcurrentQueue queue(5); // Create a concurrent queue with a capacity of 5
    AtomicCounter counter; // Create an atomic counter

    std::vector<std::thread> producers; // Vector to hold producer threads
    for (int i = 0; i < 3; ++i) {
        producers.emplace_back(producer, std::ref(queue), std::ref(counter), i); // Create 3 producer threads
    }

    std::thread consumer_thread(consumer, std::ref(queue), std::ref(counter), 3); // Create a consumer thread

    for (auto& producer : producers) {
        producer.join(); // Wait for all producer threads to finish
    }

    consumer_thread.join(); // Wait for the consumer thread to finish

    std::cout << "Total items produced: " << counter.get() << "\n"; // Output the total items produced
    return 0;
}
```
- A `ConcurrentQueue` with a capacity of 5 and an `AtomicCounter` are instantiated.
- Three producer threads are created, each producing items and pushing them to the queue.
- One consumer thread consumes the produced items.
- The main thread waits for all producer and consumer threads to finish and then outputs the total number of items produced.

### Conclusion
This code demonstrates a classic producer-consumer scenario with thread synchronization using mutexes and condition variables in C++. The use of an atomic counter allows for safe incrementing of a shared count across multiple threads without requiring additional locking mechanisms. The concurrent queue ensures that producers and consumers can operate concurrently without stepping on each other's toes.

