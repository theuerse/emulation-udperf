#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <sstream>

#define UINT32_LENGTH sizeof(uint32_t)

using boost::asio::ip::udp;
using namespace std;

enum { max_length = 1470 };
string logpath;
stringstream logBuffer;
udp::socket* sck;

void signalHandler(int signum);
void flushLogBuffer();

int main(int argc, char* argv[])
{
    if(argc != 7){
        std::cerr << "Usage: " << argv[0] << "  tgt-IP    tgt-port   bitrate[kbps]  runtime[s]  own-port  logfile-path" << std::endl;
        std::cerr << "Usage: " << argv[0] << " 127.0.0.1    6000        1000           99         6001   ./udperf_sender-0_1.log" << std::endl;
        std::cerr << "own-port == 0 => select free port" << std::endl;
        exit(EXIT_FAILURE);
    }

    unsigned int bitrate = boost::lexical_cast<unsigned int>(argv[3]);

    boost::asio::io_service io;
    boost::asio::deadline_timer timer(io);

    boost::posix_time::time_duration tgt_interval = boost::posix_time::microseconds((8.0 * max_length) / (bitrate * 1000.0) * 1000000);
    boost::posix_time::time_duration interval;
    std::cout << "interval: " << to_simple_string(tgt_interval) << std::endl;

    // initialize logfile
    logpath = string(argv[6]);
    logBuffer << "interval: " << to_simple_string(tgt_interval) << '\n';
    logBuffer << "time\tseqnr\n";

    uint32_t seqnr = 0;
    boost::posix_time::ptime now;
    boost::posix_time::ptime prev;
    boost::posix_time::ptime endTime = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::seconds(boost::lexical_cast<unsigned int>(argv[4]));

    char data[max_length];

    udp::socket s(io, udp::endpoint(udp::v4(), std::atoi(argv[5])));
    udp::resolver resolver(io);
    udp::endpoint endpoint = *resolver.resolve({udp::v4(), argv[1], argv[2]});

    // register signals indicating (probably) impending execution end
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    prev = boost::posix_time::microsec_clock::universal_time();
    while(((now = boost::posix_time::microsec_clock::universal_time()) < endTime)){
        // determine start of next interval
        interval -= (now - prev - tgt_interval);
        timer.expires_at(now + interval);

        // fill in current packet-count and date into data
        memcpy(data,&seqnr,UINT32_LENGTH);

        // send udp-packet
        s.send_to(boost::asio::buffer(data,max_length), endpoint);
        logBuffer << to_simple_string(now) << '\t' << seqnr << '\n';
        ++seqnr;

        // wait synchronously
        timer.wait();
        prev = now;
    }
    s.close();
    flushLogBuffer();
    cout << "Logfile written\n\n";
    std::cout << "packets sent: " << seqnr << std::endl;
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