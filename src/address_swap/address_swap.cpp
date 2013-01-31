#include <iostream>
#include <istream>
#include <string>
#include <cstring>

int main() {
    char ipv4header_buffer[60];
    char data_buffer[1 << 16];
    char source_addr[4];
    
    while(1)
    {        
        std::cin.read(&ipv4header_buffer[0], 1);
        const int BYTES_PER_WORD = 4;
        int ipv4header_length = (ipv4header_buffer[0] & 0x0f) * BYTES_PER_WORD;
//        std::cerr << "Ipv4header length (bytes): " << ipv4header_length << std::endl;
        std::cin.read(&ipv4header_buffer[1], ipv4header_length-1);
    
//        for(int i = 0; i < ipv4header_length; ++i)
//            std::cerr << std::dec <<  i << ": " << std::hex << ((unsigned int)ipv4header_buffer[i] & 0xff) << std::endl;

        unsigned int total_length = ((static_cast<unsigned int>(ipv4header_buffer[2]) & 0xff) << 8) | (ipv4header_buffer[3] & 0xff);
//        std::cerr << std::dec << "Total length (bytes): " << total_length << std::endl;

        unsigned int data_length = total_length-ipv4header_length;
        std::cin.read(&data_buffer[0], data_length);
    
//        for(int i = 0; i < data_length; ++i)
//            std::cerr << std::dec <<  i << ": " << std::hex << (int)data_buffer[i] << std::endl;

        // write source as dest
        memcpy(&source_addr[0], &ipv4header_buffer[12], 4);
        memcpy(&ipv4header_buffer[12], &ipv4header_buffer[16], 4);
        memcpy(&ipv4header_buffer[16], &source_addr[0], 4);

        std::cout.write(&ipv4header_buffer[0], ipv4header_length);
        std::cout.write(&data_buffer[0], data_length);
        std::cout.flush();        
    }    
}
