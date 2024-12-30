#ifndef ABX_CLIENT_H
#define ABX_CLIENT_H

#include <string>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <fstream>
#include <set>
#include <../json-3.11.3/single_include/nlohmann/json.hpp>  // For JSON handling

// Using the nlohmann::json library
using json = nlohmann::json;

class ABXClient {


//public:
      //std::vector<std::vector<uint8_t>> packets;
    //std::set<int> receivedSeqs;
public:
    ABXClient(const std::string& hostname, uint16_t port);
    ~ABXClient();

    // Connect to the server
    bool connectToServer();

    // Send request to the server
    bool sendRequest(uint8_t callType, uint8_t resendSeq = 0);

    // Receive and process server response
    void processResponse();

    // Close the connection
    void closeConnection();
 //std::vector<std::vector<uint8_t>> packets;
   // std::set<int> receivedSeqs;
private:
    // Helper function to convert byte order to big endian
    int32_t toBigEndian32(int32_t value);

    // Server connection details
    std::string hostname_;
    uint16_t port_;
    int socket_fd_;
uint8_t callType;
    // Helper functions to send/receive data
    bool sendData(const std::vector<uint8_t>& data);
    bool receiveData(std::vector<uint8_t>& buffer, size_t size);


    // Function to handle packet processing and generate JSON output
    void handlePackets(const std::vector<std::vector<uint8_t>>& packets);

    // Function to handle missing sequence numbers
    //void handleMissingSequences(const std::vector<std::vector<uint8_t>>& packets);
    void handleMissingSequences(const std::set<int>& receivedSeqs, const std::vector<std::vector<uint8_t>>& packets);

    // Function to write the JSON data to a file
    //void writeJsonToFile(const json& packetsJson);
    void writeJsonToFile(const std::vector<json>& packetsJson);
};

#endif // ABX_CLIENT_H

