#include <iostream>
#include <istream>
#include <string>
#include <cstring>
#include <boost/asio.hpp>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <ctime>

using boost::asio::ip::udp;

int main(int argc, char* argv[]) {
    if (argc != 4)
    {
        std::cerr << "Usage: drc_blackout <dest-host> <port> <blackout-file.csv>" << std::endl;
        return 1;
    }
    
    char ipv4header_buffer[60];
    char data_buffer[1 << 16];
    char source_addr[4];
    boost::asio::io_service io_service;
    udp::endpoint receiver_endpoint(boost::asio::ip::address_v4::from_string(argv[1]), atoi(argv[2]));

    udp::socket socket(io_service);
    socket.open(udp::v4());


    std::vector<int> blackout_intervals;
    std::ifstream blackoutfile(argv[3]);
    
    const int good_comms_length = 1;        
    int l = -1;
    try
    {
        if(!blackoutfile.is_open())
            throw(std::runtime_error("Could not open file for reading."));
        
        std::string line;
        while (std::getline(blackoutfile, line))
        {
            ++l;
            if(l == 0)
                continue; // header line
                           
            std::vector<std::string> svec;
            boost::split(svec, line, boost::is_any_of(","));
            blackout_intervals.push_back(boost::lexical_cast<int>(svec.at(1))+good_comms_length);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Failed to parse: " << argv[3] << " blackout file at line " << l << ". Reason: " << e.what() << std::endl;
        exit(1);
    }

    std::cout << "Blackout schedule read in successfully: " << std::endl;
    for(std::vector<int>::const_iterator it = blackout_intervals.begin(), end = blackout_intervals.end(); it != end; ++it)
        std::cout << *it << ",";
    std::cout << std::endl;
    
    std::time_t previous_time = time(0);
    
    std::vector<int>::const_iterator blackout_it = blackout_intervals.begin();
    
    while(1)
    {        
        std::cin.read(&ipv4header_buffer[0], 1);
        const int BYTES_PER_WORD = 4;
        int ipv4header_length = (ipv4header_buffer[0] & 0x0f) * BYTES_PER_WORD;
        std::cin.read(&ipv4header_buffer[1], ipv4header_length-1);    
        unsigned int total_length = ((static_cast<unsigned int>(ipv4header_buffer[2]) & 0xff) << 8) | (ipv4header_buffer[3] & 0xff);
        unsigned int data_length = total_length-ipv4header_length;
        std::cin.read(&data_buffer[0], data_length);


        const int udp_header_size = 8;

        std::time_t current_time = time(0);
        int time_diff = difftime(current_time,previous_time);

//        std::cout << "previous_time: " << previous_time << ", current_time: " << current_time << ", time_diff: " << time_diff << std::endl;
        

        while(time_diff >= *blackout_it) // past the blackout interval
        {
            previous_time += *blackout_it;
            blackout_it++;
            if(blackout_it == blackout_intervals.end())
            {
                std::cout << "Resetting blackout schedule to beginning" << std::endl;
                blackout_it = blackout_intervals.begin();
            }
            time_diff = difftime(current_time,previous_time);
        }
        
        if(time_diff < good_comms_length) // in the second following blackout, i.e. good comms        
        {
            //   std::cout << "Good comms @ " << current_time << std::endl;
            socket.send_to(boost::asio::buffer(&data_buffer[udp_header_size], data_length-udp_header_size), receiver_endpoint);
        }
    }    
}
