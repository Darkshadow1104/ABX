#include "ABXClient.h"
#include <iostream>
#include <thread>
int main() {
    ABXClient client("127.0.0.1", 3000); // Change IP as necessary
/*
    if (!client.connectToServer()) {
        return 1;
    }
*/
    std::thread responseThread(std::bind(&ABXClient::connectToServer, &client));
    //std::thread responseThread(ABXClient::connectToServer);
    responseThread.join();
/*
    // Send request to stream all packets
    if (!client.sendRequest(1)) {
        return 1;
    }
*/

/*

    uint8_t callType;
    std::cout << "Enter callType (1 for Stream All Packets, 2 for Resend Packet): ";
    std::cin >> callType;
*/
    
    // For callType 2, also ask for the sequence number to resend
    
    uint8_t resendSeq = 0;
    /*
    if (callType == 2) {
        std::cout << "Enter sequence number to resend: ";
        std::cin >> resendSeq;
    }*/
    /*
  // Send a request to stream all packets (Call Type 1)
    std::cout << "Requesting to stream all packets...\n";
    if (!client.sendRequest(1)) {
        std::cerr << "Failed to send request to stream all packets. Exiting...\n";
        client.closeConnection();
        return 1;
    }
*/

     // Step 3: Send request to the server based on the callType
    if (!client.sendRequest(1, resendSeq)) {
        std::cerr << "Failed to send request\n";
       // client.closeConnection();
        return 1;
    }
    // Process server response
    //std::thread t(&ABXClient::processResponse);
    client.processResponse();


//    client.handleMissingSequences(receivedSeqs, packets);


   // client.handlePackets(packets);
    
//t.join();
    // Close the connection
    //client.closeConnection();
/*std::thread responseThread([&client]() {
        client.processResponse();
    });
 if (!client.sendRequest(1, resendSeq)) {
        std::cerr << "Failed to send request\n";
       // client.closeConnection();
        return 1;
    }
    // Continue doing other tasks in the main thread
    responseThread.join();*/
    return 0;
}

