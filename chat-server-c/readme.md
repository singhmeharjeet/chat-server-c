#### 1. Four Threads fork from the main thread, to handle their respective tasks
```c
// Threads
pthread_t keyboardThread, udpSenderThread, udpReceiverThread, screenOutputThread;

// Corresponding functions
void* udp_sender_thread(void* arg);
void* udp_receiver_thread(void* arg);
void* keyboard_input_thread(void* arg);
void* screen_output_thread(void* arg);
```
#### 2. They share the following variables
```c
// Hold the message buffer(string) in  a shared list to avoid duplication
List* messagesToSend;
List* messagesToDisplay;

// Locks for multithreading
pthread_mutex_t sendListMutex, displayListMutex;
pthread_cond_t sendListCond, displayListCond;
``` 

#### 3. Internally the List data structure used doesn't make any system calls. It has a limit of 100 pointers it can store, across all the lists combined.
```c
#define LIST_MAX_NUM_HEADS 10
#define LIST_MAX_NUM_NODES 100
```

### Socket Creation
* Gets the IP address from the hostname provided.
* Binds the specified port to the newly created socket. Now, you can send and recive messages from this socket. 
* The whole programme uses a reference to the socket a.k.a the `socket_file_descriptor`, so this functions returns the file descriptor of that socket.

### The Sender Thread
* It waits for the `messagesToSend` list's count to change.
* Once it knows there is a message to send, it writes it to the socket.
* It also checks if the sent message is `"!"` to find out when to terminate.
* Each cycle it aslo checks if a message to terminate was received from the host.

### The Reciver Thread
* It reads data (message) from the `receiver_socket_fd` and puts it into a buffer for further processing.
* If the received message contains a termination request then it signals to the `udpSenderThread` and ` screenOutputThread` to join into the main.
* It also flags the `REMOTE_TERMINATION` to inform to the keyboard thread to stop waiting for the input from the user.
* A function `select()` is used to call `recvFrom()` only when data is available to be read from the socket. This is done to check for a `LOCAL_TERMINATION` request from the `keyboard_thread`. 
* If `select()` was not used then the `recvFrom()` will block the execution, waiting to read input and input will never arrive because `udpSenderThread` might have sent a termination request to the other end.
* Hence it has to check for `LOCAL_TERMINATION` every cycle. Therefore `select()` is used.

### The Output Thread
* It checks `displayList` and prints out if a message is waiting to be displayed.
* Also listens to the termination request from `keyboardThread` and `receiverThread`.

### The Keyboard Thread
* It reads data from the console and puts it into a buffer, and pushes the buffer to the `sendMessageList` for furthre use.
* If the buffer (message) contains a termination request then it signals to the `udpSenderThread` and ` screenOutputThread` to join into the main.
* It also flags the `LOCAL_TERMINATION` to inform to the `receiver_thread` to stop waiting for the input from the remote connection.
* A function `select()` is used to call `fgets()` only when data is available to be read from the socket. 
* This is done to check for a `REMOTE_TERMINATION` request from the `receiver_thread`. If `select()` was not used then `fgets()` will block the execution of the thread, waiting to read input and input will never arrive because `udpReceiverThread` might have received a termination request from the other end.
* Hence it has to check for `REMOTE_TERMINATION` every cycle. Therefore `select()` is used.


### At the end, i.e. after a termination request 
1. All the threads are joined back into the main.
2. Allocated memory is freed.
3. The user is informed of the termination request.
4. `EXIT_SUCCESS`
