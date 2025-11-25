# C++ Multi-Client Chat Server

A TCP-based chat server demonstrating efficient multi-client handling using `select()` for I/O multiplexing.

## 🎯 Project Overview

Built to learn C++ networking and server-side systems programming. Implements a real-time chat server where multiple clients can connect simultaneously and exchange messages.

## 🏗️ Technical Architecture

### Server Design
- **I/O Multiplexing**: Uses `select()` to handle multiple clients in a single thread
- **Event-Driven**: Non-blocking architecture that scales efficiently
- **State Management**: Tracks connected clients and usernames using STL containers

### Key Concepts Demonstrated
- TCP socket programming (socket, bind, listen, accept)
- `select()` for monitoring multiple file descriptors
- Client state management with `std::vector` and `std::map`
- Broadcasting messages to multiple clients
- Handling client connections and disconnections

## 🚀 Features

- Real-time multi-client chat
- Username registration on connect
- Broadcast messages to all connected clients
- Join/leave notifications
- Graceful disconnect handling

## 💻 Technical Stack

- **Language**: C++
- **Networking**: Berkeley sockets API
- **I/O Model**: select() (multiplexing)
- **Client Threading**: POSIX threads for concurrent send/receive

## 📦 How to Build & Run

### Compile
```bash
# Server (no threading needed - uses select)
g++ server.cpp -o server

# Client (uses threads for send/receive)
g++ client.cpp -o client -pthread
```

### Run
```bash
# Terminal 1: Start server
./server

# Terminal 2 to n: Connect clients
./client
```

## 🧠 What I Learned

### The Journey
I started by following a basic socket tutorial, got a simple echo server working, 
then expanded it to handle multiple clients using threading. After realizing 
threading didn't scale well, I looked into how to more efficiently handle thousands 
of connections, in the way that large servers do, and discovered select(). Refactoring 
from threading to select() taught me more about networking than the initial implementation.

### Key Concepts
- **TCP fundamentals**: Understanding the difference between socket(), bind(), 
  listen(), and accept() by actually implementing them
- **select() vs threading**: Threading is intuitive (one thread per client) but 
  doesn't scale. select() lets one thread monitor hundreds of sockets efficiently
- **State management challenges**: Tracking which client is which, handling 
  disconnects, cleaning up resources properly
- **Buffer management**: Learning why memset() is critical - spent time 
  debugging messages getting concatenated before realizing I wasn't clearing buffers

#### Architecture Decisions
- **Threading → select() refactor**: I initially built this with threading (one thread 
  per client) because it was conceptually simpler - each client gets its own handler. 
  After getting it working, I learned about select() and realized it's more scalable. 
  The refactor taught me about event-driven I/O and why game servers use this pattern. The threading version is still in the code (commented) as reference.
- **State management**: Used map for O(1) username lookups by socket descriptor
- **Broadcast efficiency**: Loop through client vector once per message

## 🔧 Challenges & Solutions

### Message Concatenation Bug
**Problem**: Messages were getting mixed together - "hi" would appear as "hillo"  
**Cause**: Not clearing buffer between reads, so old data contaminated new messages  
**Solution**: Added `memset(buffer, 0, sizeof(buffer))` before each read

### Multi Client Connection
**Problem**: Only one client was able to connect despite thread implentation 
**Cause**: Both thread logic, and compiling with threads  
**Solution**: Changed logic to create thread properly and compiled with thread

### Username Prompt
**Problem**: Username prompt appeared, but would not save the name  
**Cause**: The data was stored, but not used in a way for clients to see  
**Solution**: Implemented leaving server and chat logic to display the username from map

### C++ Specific
- Working with file descriptors and fd_set
- STL containers for real-time data management
- String handling and buffer management
- Memory safety with proper cleanup

## 🎮 Relevance to Game Servers

This project demonstrates core concepts used in online game servers:
- Managing multiple concurrent connections
- Broadcasting state updates to clients
- Handling player join/leave events
- Efficient I/O patterns for real-time systems

## 📈 Potential Enhancements

- Add rooms/channels
- Implement simple game logic (turn-based game)
- Add reconnection handling
- Integrate database for persistent chat history
- Implement client prediction patterns

## 🔗 Related Projects

- [Authentic Conversation Starter](https://github.com/lucasbwein/Conversation-Starter) - React app with state management
- [Replace the Urge](https://github.com/lucasbwein/Urge-Replacer) - Behavioral tracking with analytics

---

**Built as part of learning systems programming for game development.**