#include <iostream>
#include <sstream>
#include <boost/program_options.hpp>
#include <fstream>
#include "shared_structs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include "err.h"
#include <boost/algorithm/string.hpp>
#define N 100
#define BUFFER_SIZE   2000
#define QUEUE_LENGTH     5
using namespace boost::program_options;
using namespace boost::algorithm;
using std::string;
using std::cout;
using std::cin;
using std::getline;

const string HELLO = "HELLO00000";
const string GOOD_DAY = "GOOD_DAY00";
const string LIST = "LIST000000";
const string MY_LIST = "MY_LIST000";
const string CONNECT_ME = "CONNECT_ME";
const string DEL = "DEL0000000";
const string ADD = "ADD0000000";
uint64_t cmd_seq = 1;

void string_to_char_with_zeros(char cmd[10], string &s){
    int n = s.length();
    for(int i=0; i<n; i++){
        cmd[i] = s[i];
    }
    for(int i = n; i <10; i++){
        cmd[i] = '\0';
    }
}
void prepare_to_send(SIMPL_CMD &packet, char cmd[10], char data[]){
    for(int i = 0; i < 10; i++){
        packet.cmd[i] = cmd[i];
    }
    packet.data = data;
    packet.cmd_seq = cmd_seq;

}
void prepare_to_send_param(CMPLX_CMD &packet, uint64_t param, char cmd[10], char data[]){
    for(int i = 0; i < 10; i++){
        packet.cmd[i] = cmd[i];
    }
    packet.data = data;
    packet.cmd_seq = cmd_seq;
}

void on_timeout(int timeout){
    if(timeout> 300 || timeout <= 0){
        throw std::invalid_argument("Timeout value specified by -t or --TIMEOUT must be between 1 and 300");
    }
}
int main(int argc, const char *argv[]) {
    uint16_t port;
    int timeout;
    string addr, savedir;
    struct ip_mreq group;
    try{
        options_description desc{"Options"};
        desc.add_options()
            ("MCAST_ADDR,g",value<string>(&addr)->required(), "adress")
            ("CMD_PORT,p", value<uint16_t>(&port)->required(), "port")
            ("OUT_FLDR,o", value<string>(&savedir)->required(), "out folder")
            ("TIMEOUT,t", value<int>(&timeout)->default_value(5)->notifier(on_timeout), "timeout");
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);
    }
    catch (const error &ex) {
        std::cerr << ex.what() << '\n';
        exit(0);
    }
    catch(std::invalid_argument &ex) {
        std::cerr << ex.what() << '\n';
        exit(0);
    }

    string command;
    string param;

    //TODO debug stuff
    cout << addr << "\n";
    cout << " port " << port << "\n";
    cout << "out: " << savedir<<"\n";
    cout << "timeout" << timeout << "\n";
    cout <<"ready to read: \n";
    string line;

    //TODO get ready with shit
    //server variables
    char buffer[BUFFER_SIZE];
    ssize_t len, snd_len;

    struct sockaddr_in client_address[N];
    socklen_t client_address_len[N];
    struct sockaddr_in localSock;
    struct pollfd fds[N];

    //initializing pollin
    for(int i=0; i<N; i++){
        fds[i].fd = -1;
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    //new connections on fds[0]
    fds[0].fd = socket(PF_INET, SOCK_DGRAM, 0); // creating IPv4 UDP socket
    if (fds[0].fd < 0)
        syserr("socket");
    int sock = fds[0].fd;

/* Enable SO_REUSEADDR to allow multiple instances of this */
/* application to receive copies of the multicast datagrams. */
    int reuse = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
        syserr("secksockopt: reuse");
    }

    //binding port number with ip address
    memset((char *) &localSock, 0, sizeof(localSock));
    localSock.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    localSock.sin_port = htons(port); // listening on port PORT_NUM
    localSock.sin_family = AF_INET; // IPv4

    if(bind(sock, (struct sockaddr*)&localSock, sizeof(localSock)) < 0){
        syserr("bind");
    }

    //join to group
    group.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(addr.c_str(), &group.imr_multiaddr) == 0)
        syserr("inet_aton");
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&group, sizeof group) < 0)
        syserr("setsockopt");



    while(getline(cin, line)){
        string a, b;
        std::istringstream iss(line);
        if(!(iss >> a)){
            continue;
        }
        else{
            to_lower(a);
        }
        if(!(iss >> b)){
            //only discover, exit, search are OK
            if(a == "discover"){
                //TODO discover
                cout << a;

                char buffer[256];
                ssize_t rcv_len;
                /* czytanie tego, co odebrano */
                for (int i = 0; i < 30; ++i) {
                    rcv_len = read(sock, buffer, sizeof buffer);
                    if (rcv_len < 0)
                        syserr("read");
                    else {
                        printf("read %zd bytes: %.*s\n", rcv_len, (int)rcv_len, buffer);
                    }
                }




            }
            else if(a == "exit"){
                //TODO exit
                exit(0);
            }
            else if(a == "search"){
                //TODO search
                cout <<"performing search..\n";
            }
            else{
                cout << a << " is unrecognized command or requires parameter\n";
            }
        }
        else{
            if(a == "fetch"){
                //TODO fetch
            }
            else if(a == "search"){
                //TODO search with argument
            }
            else if(a == "upload"){
                //TODO upload
            }
            else{
                cout << a << " is unrecognized command or requires to be without parameters\n";
            }
            //only fetch, search, upload are correct
        }
    }


    return 0;
}