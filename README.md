# 🏭 TUMZ Warehouse - Furniture Order Management System

A multithreaded warehouse management system built in C, demonstrating advanced operating system concepts including producer-consumer problem, semaphores, mutex locks, priority scheduling, and concurrent processing.

---

## ⚛️ Built With

- **C (POSIX)** — Multithreading and synchronization
- **pthread Library** — Thread management
- **Semaphores** — Resource synchronization
- **Mutex Locks** — Critical section protection
- **File I/O** — Order and log management

---

## 👨‍💻 Developers

| Name | Role | Institution |
|------|------|-------------|
| Tasbiha Nasir | Developer | FAST University |
| Uzair Hussain | Developer | FAST University |
| Madiha Aslam | Developer | FAST University |
| Zain Saqib | Developer | FAST University |

**Course**: Operating Systems  
**Project Type**: Concurrent Programming & Synchronization

---

## 🧩 System Overview

TUMZ Warehouse implements the classic **Producer-Consumer Problem** with priority scheduling. The system manages furniture orders (Chairs, Sofas, Beds) using separate priority and normal queues, demonstrating real-world application of OS concepts like thread synchronization, deadlock prevention, and resource management.

---

## 🗄️ Core Features

### 🔄 Producer-Consumer Architecture
- **3 Producer Threads**: One for each item type (Chair, Sofa, Bed)
- **3 Consumer Threads**: Process items from both queues
- **Dual Buffer System**: Separate priority and normal queues
- **Priority Scheduling**: Orders ≤10 items get priority treatment

---

### 📊 Synchronization Mechanisms

#### Semaphores (4 total)
- `semEmptyNormal` - Tracks empty slots in normal buffer
- `semFullNormal` - Tracks filled slots in normal buffer
- `semEmptyPriority` - Tracks empty slots in priority buffer
- `semFullPriority` - Tracks filled slots in priority buffer

#### Mutex Locks (3 total)
- `mutexNormalBuffer` - Protects normal buffer operations
- `mutexPriorityBuffer` - Protects priority buffer operations
- `orderLock` - Protects order progress tracking

---

### 🎯 Key Operating System Concepts

#### 1. **Multithreading**
- Concurrent producer and consumer execution
- Thread creation using `pthread_create()`
- Thread synchronization using `pthread_join()`

#### 2. **Semaphores**
- Binary semaphores for signaling
- Counting semaphores for buffer management
- `sem_wait()` and `sem_post()` operations

#### 3. **Mutex Locks**
- Critical section protection
- Deadlock prevention
- Race condition avoidance

#### 4. **Priority Scheduling**
- Two-level queue system
- Priority queue serviced first
- Starvation prevention with fallback to normal queue

#### 5. **Buffer Management**
- Circular buffer implementation (size: 10)
- Front and rear pointer management
- Count-based occupancy tracking

---

## 📁 System Architecture

### Data Structures

```c
struct Buffer {
    int buffer[BUFFER_SIZE];    // Circular buffer array
    int front;                   // Dequeue position
    int rear;                    // Enqueue position
    int count;                   // Current items count
};

struct Order {
    int order_id;
    int quantity[3];             // Chair, Sofa, Bed
    bool isPriority;
    char creation_time[64];
    char completion_time[64];
};
```

### Order Encoding
- Each item encoded as: `orderIndex * 10 + itemType`
- Example: Order 5, Sofa → `52`
- Allows tracking which order and item type in single integer

---

## 🚀 Features & Operations

### 1. **Create New Orders**
- Manual order entry with validation
- Duplicate ID prevention
- Automatic priority assignment (≤10 items = priority)
- Timestamp recording (creation time)
- Data persistence in `orders.txt`

### 2. **Start Processing Orders**
- Spawns producer and consumer threads
- Producers add items to appropriate queue
- Consumers process items with priority preference
- Automatic completion detection
- Completion timestamp logging

### 3. **View Current Orders**
- Display all orders with quantities
- Show completion status
- Real-time order tracking

### 4. **View Logs**
- Formatted table display
- Shows creation and completion times
- Persistent log storage in `logs.txt`

### 5. **Check Order Status**
- Track production progress
- Identify pending orders
- Monitor not-yet-started orders

---

## ⚙️ File Structure

```
TUMZ-Warehouse/
│
├── code1.c                    # Main program
│
├── Data Files/
│   ├── orders.txt             # Current orders
│   └── logs.txt               # Completed order logs
│
└── README.md
```

---

## 🔄 System Workflow

### Producer Thread Logic
```
For each order:
    For each item in order:
        If priority order:
            Wait for empty slot in priority queue
            Lock priority buffer
            Add item to priority queue
            Unlock priority buffer
            Signal full priority queue
        Else:
            Wait for empty slot in normal queue
            Lock normal buffer
            Add item to normal queue
            Unlock normal buffer
            Signal full normal queue
```

### Consumer Thread Logic
```
While orders exist:
    Try to consume from priority queue first
    If priority queue empty:
        Try to consume from normal queue
    If both empty:
        Wait and retry
    
    Decode item (order ID + item type)
    Update order progress
    If order complete:
        Log completion with timestamp
```

---

## 💻 Compilation & Execution

### Prerequisites
- GCC compiler with pthread support
- POSIX-compliant system (Linux/Unix/macOS)
- Semaphore support

### Compilation

```bash
# Linux/Unix
gcc code1.c -o tumz -lpthread

# macOS
gcc code1.c -o tumz -lpthread

# Run
./tumz
```

### Sample Session

```
***************************
---------------------------
WELCOME TO TUMZ WARE HOUSE 
---------------------------
***************************

===== MENU =====
1. Create new orders.
2. Start processing orders.
3. View current orders.
4. View Logs
5. Check Order Status.
6. Exit
Select the option: 1

Enter Number of orders: 2

Order #1 id: 101
  Chairs: 5
  Sofas : 3
  Beds  : 2

Order #2 id: 102
  Chairs: 15
  Sofas : 10
  Beds  : 5

Orders saved.

Select the option: 2
[Producer-Chair] Pushed PRIORITY Chair (Order 101)
Priority Queue: [1010]

[Consumer] Consumed PRIORITY item 1010
Priority Queue: []
...
Order 101 is COMPLETE (Chair, Sofa, Bed)
```

---

## 📊 Data Format Examples

### orders.txt
```
Order 101: Chair=5, Sofa=3, Bed=2
Order 102: Chair=15, Sofa=10, Bed=5
```

### logs.txt
```
OrderNumber=101 Chair=5 Sofa=3 Bed=2 CreationTime=2024-01-15 10:30:45 CompletionTime=2024-01-15 10:35:22
OrderNumber=102 Chair=15 Sofa=10 Bed=5 CreationTime=2024-01-15 10:31:00 CompletionTime=2024-01-15 10:42:18
```

---

## 🔒 Synchronization Features

### Deadlock Prevention
- Consistent lock ordering
- Timeout mechanisms
- Error handling on lock failures

### Race Condition Avoidance
- Mutex protection on shared data
- Atomic operations on critical sections
- Order progress tracking protection

### Starvation Prevention
- Priority queue checked first
- Fallback to normal queue ensures all orders processed
- Fair scheduling among consumers

---

## 🎯 Operating System Concepts Demonstrated

| Concept | Implementation |
|---------|---------------|
| **Multithreading** | pthread_create, pthread_join |
| **Semaphores** | sem_init, sem_wait, sem_post, sem_trywait |
| **Mutex Locks** | pthread_mutex_lock, pthread_mutex_unlock |
| **Critical Section** | Order progress updates, buffer access |
| **Producer-Consumer** | Dual-buffer system with synchronization |
| **Priority Scheduling** | Two-level queue with priority preference |
| **Circular Buffer** | Ring buffer with front/rear pointers |
| **Thread Synchronization** | Semaphore and mutex coordination |
| **Signal Handling** | Timeout handler for graceful shutdown |

---

## 🚨 Error Handling

- **Buffer Overflow Prevention**: Semaphore-based capacity control
- **Invalid Input Validation**: Robust input checking with retry
- **Duplicate Order ID Detection**: Prevents ID conflicts
- **Lock Failure Handling**: Error messages and graceful exit
- **File Operation Errors**: Perror-based error reporting
- **Audio Alerts**: System beep on errors and warnings

---

## 📈 Performance Features

- **Concurrent Processing**: Multiple producers/consumers work simultaneously
- **Priority Optimization**: Important orders processed faster
- **Non-blocking Checks**: `sem_trywait()` for priority testing
- **Sleep-based Delays**: Simulate real production time
- **Efficient Buffer Usage**: Circular buffer maximizes space

---

## 🔮 Future Enhancements

- [ ] Dynamic thread pool sizing
- [ ] Configurable buffer sizes
- [ ] GUI dashboard for monitoring
- [ ] Real-time statistics display
- [ ] Network-based distributed processing
- [ ] Database integration (SQLite)
- [ ] Advanced scheduling algorithms (Round Robin, FCFS)
- [ ] Resource utilization metrics
- [ ] Deadlock detection algorithms
- [ ] Load balancing across multiple warehouses

---

## 🐛 Known Limitations

- Fixed buffer size (10 items)
- Fixed thread count (3 producers, 3 consumers)
- No persistent state across program restarts
- Console-based interface only
- Single warehouse limitation

---

## 📚 Learning Outcomes

This project demonstrates:
- ✅ Thread creation and management
- ✅ Synchronization primitives usage
- ✅ Critical section protection
- ✅ Producer-consumer problem solution
- ✅ Priority scheduling implementation
- ✅ Deadlock and race condition avoidance
- ✅ Buffer management techniques
- ✅ Real-world OS concept application

---



## 📄 License

Educational project created for FAST University Operating Systems course.

---

## 🏁 Conclusion

TUMZ Warehouse demonstrates practical implementation of fundamental operating system concepts in a real-world warehouse management scenario, showcasing thread synchronization, resource management, and concurrent programming principles.

---

**💡 Developed By**

Tasbiha Nasir,Madiha Aslam, Uzair Hussain, and Zain Saqib  
*Operating Systems Project*  
*FAST University*
