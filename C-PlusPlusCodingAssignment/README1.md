This project implements a daemon (server) and a CLI (client) in C++ that communicate via TCP sockets.The system supports multiple clients concurrently, allows inserting/deleting/printing/finding numbers, and maintains an in-memory data structure with timestamps. The daemon logs all operations to daemon.log.

Daemon
* Stores numbers with their insertion timestamps (Unix time).
* Prevents duplicates.
* Supports multiple concurrent CLI clients using threads.
* Logs all operations to daemon.log.
* Connection is closed in 60 seconds if no activity from the client side. (This also protects the thread pool from being completely exhausted, ensuring that other clients can still connect and have their requests processed.)

CLI
* Insert a number (positive integer only).
* Delete a number.
* Print all stored numbers in sorted order with timestamps.
* Delete all numbers.
* Find if a number exists.
* Exit gracefully.
* Connection is closed in 60 seconds if no activity from the client side

IPC Mechanism
We use TCP sockets for communication between CLI and Daemon.
    * Platform independent.
    * Easy to scale across multiple machines (not just same host).
    * Supports multiple clients simultaneously.
    * Robust and widely supported in C++.

Data Structure Choice is Map
Key = integer
Value = insertion timestamp

Sorted storage : ensures PRINT outputs numbers in ascending order automatically.
Unique keys : duplicates are automatically prevented.
Efficient operations:
* Insert: O(log n)
* Delete: O(log n)
* Lookup: O(log n)
* Iteration (sorted): O(n)
Timestamps stored alongside numbers allow easy retrieval of both value and metadata.

Additionally used set to ensure Ensures uniqueness.
So first value is check in the set and then inserted into the map. Additional storage is needed to avoid the searching time in the map.

Concurrency Model
The daemon uses a thread pool (default: 4 worker threads).
* Client connections are accepted and added to a task queue. Task queue is protected with queue_mutex
* Worker threads fetch connections from the queue and handle requests.
* This avoids unbounded thread creation and improves performance under load.
* Access to the shared data structure (map and set) is synchronized with data_mutex.


Prerequisites
* Linux / macOS
* g++ with C++17 support
* make

Steps to build 
make clean
make
make all  

Folder structure 
C-PlusPlusCodingAssignment/
	Makefile
	readme.md
	README1.md
	src/
		lib/
			comms.cpp
			comms.hpp
		daemon/
			daemon.cpp
		cli/
			cli.cpp
		build/        # generated after `make`
			daemon
			cli
			libcomms.dylib
			*.o
			daemon.log
*Note due to macOS the lib is .dylib

Run the daemon 
./daemon [thread_pool_size] [port] [persistent]
./daemon          # uses default pool size, port 5555
./daemon 8        # 8 threads, port 5555
./daemon 8 6000   # 8 threads, port 6000
./daemon 2 12345  # 2 threads, port 12345

Running cli
./cli [host] [port] [persistent]
./cli                # connect to 127.0.0.1:5555
./cli 127.0.0.1 6000 # connect to daemon on port 6000
./cli myserver 12345 # connect to remote daemon

Config file can be create in the project folder and can be used. Command line arguments take the priority always. Both daemon and cli uses this file.
Config file looks like
host=127.0.0.1
port=9999
persistent=true


Following is how to run this.
1. Build the project (make)
2. Create/update config files (already present in the project)
3. Start the daemon (./daemon)
4. Start multiple CLI (in a new terminal) (./cli)
5. CLI interaction example
6. daemon.log is create in the project. These logs are from the daemon only.

=== CLI Menu ===
1. Insert a number
2. Delete a number
3. Print all numbers
4. Delete all numbers
5. Find a number
6. Exit
Choice: 1
Enter a positive integer: 42
Response: Inserted 42 at 1694090000

Choice: 1
Enter a positive integer: 7
Response: Inserted 7 at 1694090005

Choice: 3
Response: 
7 inserted at 1694090005
42 inserted at 1694090000

Choice: 5
Enter a positive integer to find: 42
Response: Found 42

Choice: 2
Enter a positive integer to delete: 7
Response: Deleted 7

Choice: 4
Response: All numbers deleted

Choice: 6
Exiting CLI.
