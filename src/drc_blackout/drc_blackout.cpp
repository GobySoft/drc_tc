#include <iostream>
#include <istream>
#include <string>
#include <cstring>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

int main(int argc, char* argv[]) {
    if (argc != 3)
    {
        std::cerr << "Usage: drc_blackout <dest-host> <port>" << std::endl;
        return 1;
    }
    
    char ipv4header_buffer[60];
    char data_buffer[1 << 16];
    char source_addr[4];
    boost::asio::io_service io_service;
    udp::endpoint receiver_endpoint(boost::asio::ip::address_v4::from_string(argv[1]), atoi(argv[2]));

    udp::socket socket(io_service);
    socket.open(udp::v4());

    
    while(1)
    {        
        std::cin.read(&ipv4header_buffer[0], 1);
        const int BYTES_PER_WORD = 4;
        int ipv4header_length = (ipv4header_buffer[0] & 0x0f) * BYTES_PER_WORD;
        std::cin.read(&ipv4header_buffer[1], ipv4header_length-1);    
        unsigned int total_length = ((static_cast<unsigned int>(ipv4header_buffer[2]) & 0xff) << 8) | (ipv4header_buffer[3] & 0xff);
        unsigned int data_length = total_length-ipv4header_length;
        std::cin.read(&data_buffer[0], data_length);

        //       std::cerr << "Number bytes: " << total_length << std::endl;
        //       std::cerr << "Number header bytes: " << ipv4header_length << std::endl;
        //       std::cerr << "Number data bytes: " << data_length << std::endl;

        const int udp_header_size = 8;
        
        socket.send_to(boost::asio::buffer(&data_buffer[udp_header_size], data_length-udp_header_size), receiver_endpoint);
    }    
}
