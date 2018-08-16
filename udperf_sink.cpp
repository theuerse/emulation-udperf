#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#define UINT32_LENGTH sizeof(uint32_t)

using namespace std;
using boost::asio::ip::udp;

enum { max_length = 1470 };
string logpath;
stringstream logBuffer;
udp::socket* sck;

void signalHandler(int signum);
void flushLogBuffer();

int main(int argc, char* argv[]){
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " port  logfile-path\n";
        std::cerr << "Usage: " << argv[0] << " 6000  ./udperf_sink-0_1.log\n";
        std::cerr << "runs forever, externally limit server runtime using e.g. 'timeout 10s ...'\n";
        exit(EXIT_FAILURE);
    }

    // initialize logfile
    logpath = string(argv[2]);
    logBuffer << "time\tseqnr\tsize\n";

    uint32_t seqnr;
    size_t bytes = 0;
    boost::posix_time::ptime now;

    boost::asio::io_service io_service;
    udp::socket s(io_service, udp::endpoint(udp::v4(), std::atoi(argv[1])));
    sck = &s;
    udp::endpoint sender_endpoint;

    // register signals indicating (probably) impending execution end
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    char data[max_length];
    for(;;){
        bytes = s.receive_from(boost::asio::buffer(data, max_length), sender_endpoint); // blocks!
        now = boost::posix_time::microsec_clock::universal_time();
        memcpy(&seqnr,data,UINT32_LENGTH); // read seqnr from begin of data
        logBuffer << to_simple_string(now) << '\t' << seqnr << '\t' << bytes << '\n';
    }
}

void flushLogBuffer(){
    std::ofstream outputFile(logpath);
    if( outputFile ){
        outputFile << logBuffer.str();
        outputFile.close();
    }
    else {
        std::cerr << "Failure opening " << logpath << '\n';
        exit(1);
    }
}

void signalHandler(int signum) {
    sck->close(); // close socket
    flushLogBuffer();
    cout << "Interrupt signal (" << signum << ") received.\n";
    cout << "Logfile written\n\n";
    exit(signum);
}