/*
Multi-Client Chat Server

TCP-based chat server with I/O  multiplexing by select().
Checks activity to process the clients through a single thread,
allowing multiple concurrent clients.

Key Ideas:
- select() for multi-client handling
- Event-driven architecture
- Client state management using STL containers
*/

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <map>

// Note: Can use threading/mutex but less ineffective
// #include <thread>
// #include <mutex>
// std::mutex clients_mutex; Dont need only for threading

//Global: tracks all connected clients
std::vector<int> clients; // Stores socket descriptors
std::map<int, std::string> client_names; // Maps socket descriptor, for the usernames

/*
broadcast(): Sends the inputed message to all clients.

message - is the string to broadcast
sender_socket - the socket ID of the sender (allowing for exclusion of message)

Loops through the clients sending (send()) the message to each
*/
void broadcast(const std::string& message, int sender_socket) {
    for (int client : clients) {
        if (client != sender_socket) { // Ensures message isn't repeated to sender
            send(client, message.c_str(), message.length(), 0);
        }
    }
}

int main() {
// ------------------- Socket Setup -------------------
    // Create socket
    // uses AF_INET/IPv4 and SOCK_STREAM/TCP (stream oriented connection) for reliability
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_fd == -1) { // if returned -1 the stops and fails creation
        std::cerr << "Socket creation failed!" << std::endl;
        return 1;
    }
    
    // Configure server address
    sockaddr_in address;
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections on any local network
    address.sin_port = htons(8080); // hosts on port 8080 using htons (Host To Network Short)
    
    // Bind socket to port (using :: to specify global namespace)
    if (::bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed!" << std::endl;
        return 1;
    }
    
    // Listen for connections that are incoming
    // Second parameter catches if max queued connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed!" << std::endl;
        return 1;
    }
    
    std::cout << "Server listening on port 8080..." << std::endl;
    
    

// ------------------- SELECT() LOOP -------------------

    fd_set read_fds; // set the file descriptors to monitor to read the activity

    // Server: loops until manually stopped
    while (true) { // Accepts and handles clients through select
        
        // Clear the set (ensuring a blank slate for looping)
        FD_ZERO(&read_fds);

        // Add server socket to set, making it so it can find new connections
        FD_SET(server_fd, &read_fds);
        int max_fd = server_fd; // used in select, records highest FD number

        // Add all client sockets to set so now server can detect messages sent
        for (int client : clients) {
            FD_SET(client, &read_fds); // sets each client
            // if the client is above the max, set new max
            if (client > max_fd) max_fd = client;
        }

        // Wait for activity on ANY socket
        // block until activity in one
        // To then select()
        // Returning number of sockets currently with activity
        // Parameters:
        // max_fd, read set, write set, exception set, timeout
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) { // Calls error if nothing is selected
            std::cerr << "Select error" << std::endl;
            continue; // Attempts call again
        }

        // Checks if server socket has activity (NEW CONNECTION)
        
        // FD_ISSET is used to check for activity
        if (FD_ISSET(server_fd, &read_fds)) {
            int addrlen = sizeof(address);
            
            // accepts client through creating new socket for the connection
            int new_client = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen);

            if (new_client < 0) { // catches if not valid client
                std::cerr << "Accept failed!" << std::endl;
                continue;
            }

            // Adds the client to vector for tracking
            clients.push_back(new_client);
            std::cout << "New client connected (socket " << new_client << ")" << std::endl;
            
            // Creates prompt for entering username to clients
            std::string prompt = "Enter your username: ";
            send(new_client, prompt.c_str(), prompt.length(), 0);
        }

        // Check all clients for activity

        // More dynamic for iterating by index if deletion/disconnection occurs
        for (int i = 0; i < clients.size(); i++) {
            int client = clients[i];

            // Checks for if data is ready
            if (FD_ISSET(client, &read_fds)) {

                // Reads data from given client
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                int valread = read(client, buffer, 1024);

                // Checks for disconnection
                if (valread <= 0) { // if 0 = disctionection, 0 > means error
                    // Client disconnected
                    std::string leaving_user = client_names[client];

                    // Handles if client leaves before inputting username
                    if(leaving_user.empty()) {
                        std::cout << "Client (socket " << client << ")" << std::endl;
                    } else { // else notifies with username of disconnection
                        std::cout << leaving_user << " disconnected" << std::endl;
                        std::string leaving_msg = leaving_user + " has left the chat";
                        broadcast(leaving_msg, -1); // Broadcasts the message to all users
                        // -1 causes it to be sent to everyone
                    }

                    // Ensures closing the client and erasing from tracking
                    close(client);
                    clients.erase(clients.begin() + i); // i makes it change accordingly
                    client_names.erase(client);
                    i--; // change index after erasing

                
                } else { // For Incoming Messages
                    
                    // Client SENT Data
                    std::string message(buffer);

                    // Checks if this is their first message (or inputing username)
                    // if find() returns the end() it means non-existant in username map
                    if (client_names.find(client) == client_names.end()) {
                        
                        // This is the username, stores it in map
                        client_names[client] = message;
                        std::cout << message << " has joined the chat!" << std::endl;

                        // broadcasts new user joining to chat
                        std::string join_msg = message + " has joined the chat!";
                        broadcast(join_msg, client);
                    } else {
                        // This is for a regular chat
                        std::string username = client_names[client]; // finds username
                        std::cout << username << ": " << message << std::endl;

                        // creates message to send to other clients
                        std::string full_msg = username + ": " + message;
                        broadcast(full_msg, client);
                    }
                }
            }
        }
    }
    
    // Never used but allows for better closing of server
    close(server_fd); 
    return 0; 
}

/*
------------------- Thread Use -------------------
Another approach rather than select(), is threading.
Although scalability isn't as efficient,

Difference:
- Thead(): Bigger load as each client has its own thread
- Select(): selects the given active block, all exist in one main thread
*/

// For Threading Structure
// void handleClient(int client_socket) {
//     char buffer[1024] = {0};

//     // GETS username
//     memset(buffer, 0, sizeof(buffer));
//     int valread = read(client_socket, buffer, 1024);

//     if (valread <= 0) {
//         close(client_socket);
//         return;
//     }

//     std::string username(buffer);

//     // stores username
//     clients_mutex.lock();
//     client_names[client_socket] = username;
//     clients_mutex.unlock();

//     std::cout << username << " joined the chat!" << std::endl;

//     // ANOUNCE to others
//     std::string join_msg = username + " joined the chat!";
//     broadcast(join_msg, client_socket);

//     // Runs until stops and accepts all clients with username
//     while(true) {
//         memset(buffer, 0, sizeof(buffer));
//         valread = read(client_socket, buffer, 1024);

//         if (valread <= 0) {
//             std::cout << client_names[client_socket] << " disconnected" << std::endl;

//             // Remove from both maps
//             clients_mutex.lock();
//             clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
//             client_names.erase(client_socket);
//             clients_mutex.unlock();

//             //Announce disconnect
//             std::string leave_msg = username + " left the chat";
//             broadcast(leave_msg, -1); // sends to everyone

//             close(client_socket);
//             break;
//         }

//         std::cout << "Recieved: " << buffer << std::endl;

//         //Pushes/Broadcases to all the other clients
//         std::string message = username + " says: ";
//         message += buffer;
//         broadcast(message, client_socket);
//     }
// }

// THREADING Client Accept
// Accept clients
    // while (true) {
    //     int addrlen = sizeof(address);
    //     int client_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen);

    //     if (client_socket < 0) {
    //         std::cerr << "Accept failed!" << std::endl;
    //         continue; // tries to accept next client
    //     }

    //     std::cout << "New client connected!" << std::endl;

    //     // Add to clients list
    //     clients_mutex.lock();
    //     clients.push_back(client_socket);
    //     clients_mutex.unlock();

    //     std::thread(handleClient, client_socket).detach();

    //     // Loop resets to accept - waits for next client
    // }