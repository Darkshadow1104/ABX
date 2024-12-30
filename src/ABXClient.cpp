#include "ABXClient.h"
#include <boost/asio.hpp>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <set>
#include <../json-3.11.3/single_include/nlohmann/json.hpp> // For JSON handling
#include <fstream>
#include <thread>  // For std::thread
#include <mutex>   // For std::mutex (if needed)
using boost::asio::ip::tcp;
using json = nlohmann::json;  // Alias for JSON
int calltype = 1;
// Constructor and Destructor
ABXClient::ABXClient(const std::string& hostname, uint16_t port)
    : hostname_(hostname), port_(port), socket_fd_(-1) {}

ABXClient::~ABXClient() {

    if (socket_fd_ != -1) {
	    std::cout<<"fd: "<<socket_fd_<<std::endl;
        closeConnection();
    }
    
}

// Connect to the ABX server
bool ABXClient::connectToServer() {
    sockaddr_in server_addr;

    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    if (inet_pton(AF_INET, hostname_.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address or Address not supported\n";
        return false;
    }

    if (connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return false;
    }

    std::cout << "Connected to server\n";
    return true;
}

// Send request to the server (either Stream All Packets or Resend Packet)
bool ABXClient::sendRequest(uint8_t callType, uint8_t resendSeq) {
    std::vector<uint8_t> request(2);
    request[0] = callType;   // callType (1 byte)
    request[1] = resendSeq;  // resendSeq (1 byte) - valid only for callType 2

    return sendData(request);
}

// Helper function to send data over TCP
bool ABXClient::sendData(const std::vector<uint8_t>& data) {
	
	std::vector<uint8_t> buffer(64);
    std::vector<std::vector<uint8_t>> packets;
    ssize_t sent = send(socket_fd_, (void*) data.data(), data.size(), 0);
    if (sent < 0) {
        std::cerr << "Failed to send data nitesh\n";
        return false;
    }
    return true;
}

// Receive response from the server and process each packet
void ABXClient::processResponse() {
    std::vector<uint8_t> buffer(1024); // Each packet is of fixed size (64 bytes)
    std::vector<std::vector<uint8_t>> packets;
    std::set<int> receivedSeqs;
  bool keepReceiving = true;
    while ( keepReceiving) {
        if (!receiveData(buffer, 1024)) {
            std::cerr << "Failed to receive data\n";
	    calltype = 2;
            break;
        }

        // Extract packet fields (big endian byte order)
        std::string symbol(reinterpret_cast<char*>(&buffer[0]), 4);
        char buySell = buffer[4];
        int32_t quantity = ntohl(*reinterpret_cast<int32_t*>(&buffer[5]));  // Big-endian to host order
        int32_t price = ntohl(*reinterpret_cast<int32_t*>(&buffer[9]));
        int32_t seqNum = ntohl(*reinterpret_cast<int32_t*>(&buffer[13]));

        // Store the packet
        packets.push_back(buffer);
        receivedSeqs.insert(seqNum);

        // Print the packet data (for illustration)
        std::cout << "Symbol: " << symbol << ", Buy/Sell: " << buySell
                  << ", Quantity: " << quantity << ", Price: " << price
              << ", SeqNum: " << seqNum << "\n";
/*
	*/
    }



    handleMissingSequences(receivedSeqs, packets);
    /* 
     }
     */

    handlePackets(packets);
}

// Receive data from the server
bool ABXClient::receiveData(std::vector<uint8_t>& buffer, size_t size) {
    ssize_t received = recv(socket_fd_, buffer.data(), size, 0);
    if (received < 0) {
        std::cerr << "Failed to receive data nitesh \n";
        return false;
    } else if (received == 0) {
        std::cerr << "Server disconnected\n";
        return false;
    }

    return true;
}

// Close the connection
void ABXClient::closeConnection() {
    close(socket_fd_);
    std::cout << "Connection closed\n";
}

// Helper to convert to big-endian format
int32_t ABXClient::toBigEndian32(int32_t value) {
    return htonl(value);  // Convert host-to-network byte order (big-endian)
}

// Handle missing sequences by sending a request to resend missing packets
void ABXClient::handleMissingSequences(const std::set<int>& receivedSeqs, const std::vector<std::vector<uint8_t>>& packets) {
    std::set<int> allSeqs;

    // Assuming the last sequence number is the highest (which should not be missing)
     std::vector<int> seqVec(receivedSeqs.begin(), receivedSeqs.end());
    int lastSeq = seqVec[seqVec.size() - 1];
    //int lastSeq = receivedSeqs[receivedSeqs.size()-1];
    std::cout<<"lastSeq:"<<lastSeq<<std::endl;
    for (int seq = 1; seq <= lastSeq; ++seq) {
        allSeqs.insert(seq);
    }

    // Identify missing sequence numbers
    std::set<int> missingSeqs;
    std::set_difference(allSeqs.begin(), allSeqs.end(), receivedSeqs.begin(), receivedSeqs.end(),
                        std::inserter(missingSeqs, missingSeqs.end()));

    for (auto seq : missingSeqs) {
        std::cout << "Missing sequence: " << seq << ", requesting resend...\n";
	uint8_t byte = static_cast<uint8_t>(seq);
        std::cout << "Missing sequence size: " << sizeof(byte) <<", " <<(int)byte<< "\n";
	std::cout<<"send: "<<sendRequest(2, byte)<<std::endl;  // Request to resend the missing packet
	char buffer[1024];
        ssize_t received = recv(socket_fd_, (void*) buffer, 1024, 0); 
	std::cout<<received<<std::endl;
	
    }
}

// Handle packets and save them as JSON
void ABXClient::handlePackets(const std::vector<std::vector<uint8_t>>& packets) {
    std::vector<json> packets_json;

    // Process each packet and convert to JSON format
    for (const auto& packet : packets) {
        std::string symbol(reinterpret_cast<const char*>(&packet[0]), 4);
        char buySell = packet[4];
        int32_t quantity = ntohl(*reinterpret_cast<const int32_t*>(&packet[5]));  // Big Endian to host order
        int32_t price = ntohl(*reinterpret_cast<const int32_t*>(&packet[9]));     // Big Endian
        int32_t seq = ntohl(*reinterpret_cast<const int32_t*>(&packet[13]));      // Big Endian

        json packet_json = {
            {"symbol", symbol},
            {"buy_sell", (buySell == 'B' ? "Buy" : "Sell")},
            {"quantity", quantity},
            {"price", price},
            {"seq", seq}
        };

        packets_json.push_back(packet_json);
    }

    // Write the JSON data to a file
    writeJsonToFile(packets_json);
}

// Write the JSON data to a file
void ABXClient::writeJsonToFile(const std::vector<json>& packetsJson) {
    std::ofstream out_file("packets.json");
    if (out_file.is_open()) {
        json final_json = {{"packets", packetsJson}};
        out_file << final_json.dump(4);  // Pretty-print with an indent of 4 spaces
        out_file.close();
        std::cout << "JSON output saved to packets.json" << std::endl;
    } else {
        std::cerr << "Failed to open file for writing!" << std::endl;
    }
}
