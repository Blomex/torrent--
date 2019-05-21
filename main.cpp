#include <iostream>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
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
#include <chrono>
#define N 100
#define BUFFER_SIZE   2000
#define QUEUE_LENGTH     5
using namespace boost::program_options;
using namespace boost::algorithm;
using std::string;
using std::cout;
using std::cin;
using std::getline;
using std::vector;
using namespace std::chrono;
const string HELLO = "HELLO";
const string GOOD_DAY = "GOOD_DAY";
const string LIST = "LIST";
const string MY_LIST = "MY_LIST";
const string CONNECT_ME = "CONNECT_ME";
const string DEL = "DEL";
const string ADD = "ADD";
const string GET = "GET";
uint64_t cmd_seq = 1;
int timeout;

int prepare_to_send(SIMPL_CMD &packet, char cmd[10], const string &data){
    for(int i = 0; i < 10; i++){
        packet.cmd[i] = cmd[i];
    }
    int pom = sizeof(packet.data);
    strncpy(packet.data, data.c_str(), sizeof(packet.data));

    packet.cmd_seq = htobe64(cmd_seq);
    int i = data.length() + sizeof(packet.cmd_seq) + sizeof(packet.cmd);
    return data.length() + sizeof(packet.cmd_seq) + sizeof(packet.cmd);
}
int prepare_to_send_param(CMPLX_CMD &packet, uint64_t param, char cmd[10], const string &data){
    //TODO strncpy instead?
    for(int i = 0; i < 10; i++){
        packet.cmd[i] = cmd[i];
    }
    int pom = sizeof(packet.data);
    strncpy(packet.data, data.c_str(), sizeof(packet.data));
    packet.cmd_seq = cmd_seq;
    return data.length() + sizeof(packet.cmd_seq) + sizeof(packet.cmd) + sizeof(packet.param);
}

void on_timeout(int timeout){
    if(timeout> 300 || timeout <= 0){
        throw std::invalid_argument("Timeout value specified by -t or --TIMEOUT must be between 1 and 300");
    }
}

void perform_search(const string &s, int sock, struct sockaddr_in &remote_address){
    char cmd[10];
    set_cmd(cmd, LIST);
    SIMPL_CMD packet;

    int length = prepare_to_send(packet, cmd, s);
    if (sendto(sock, &packet, length, 0, (struct sockaddr*)&remote_address, sizeof remote_address) != length)
        syserr("write");
    CMPLX_CMD p3;
    struct sockaddr_in server;
    socklen_t len = sizeof(struct sockaddr_in);
    auto start = high_resolution_clock::now();

    while(1) {
        //TODO do poprawy
        auto end = high_resolution_clock::now();
        duration<double, std::milli> elapsed = end - start;
        if (elapsed.count() >= timeout * 1000) {
            break;
        }
        int x = recvfrom(sock, &p3, sizeof p3, 0, (struct sockaddr *) &server, &len);
        if (x < 0) {
            continue;
        }
        p3.data[x - 26] = '\0';
        //check if we got packet we are expecting
        int port = ntohs(server.sin_port);
        string ip = inet_ntoa(server.sin_addr);
        if (strncmp(p3.cmd, "MY_LIST", 8) == 0 && be64toh(p3.cmd_seq)==cmd_seq) {
            vector<string> result;
            boost::split(result, p3.data, boost::is_any_of("\n"));
            for(string &str : result){
                cout << str <<" ("<<ip<<")\n";
            }
        }
        else{
            cout <<"[PCKG ERROR]  Skipping invalid package from "<<ip<<":"<<port<<".\n";
        }
    }


}

void perform_fetch(string &s, int sock, struct sockaddr_in &remote_address){
  char cmd[10];
  set_cmd(cmd, GET);
  SIMPL_CMD packet;
  int length = prepare_to_send(packet, cmd, s);
    if (sendto(sock, &packet, length, 0, (struct sockaddr*)&remote_address, sizeof remote_address) != length)
        syserr("sendto");

}

void perform_discover(int sock, struct sockaddr_in &remote_address){
    char cmd[10];
    set_cmd(cmd, HELLO);
    SIMPL_CMD packet;
    int length = prepare_to_send(packet, cmd, "");

    if (sendto(sock, &packet, length, 0, (struct sockaddr*)&remote_address, sizeof remote_address) != length)
        syserr("sendto");
    CMPLX_CMD p3;
    struct sockaddr_in server;
    socklen_t len = sizeof(struct sockaddr_in);
    auto start = high_resolution_clock::now();
    while(1) {
        //TODO do poprawy
        auto end = high_resolution_clock::now();
        duration<double, std::milli> elapsed = end - start;
        if(elapsed.count()>=timeout*1000){
            break;
        }
        int x = recvfrom(sock, &p3, sizeof p3, 0, (struct sockaddr *) &server, &len);
        if (x < 0) {
            continue;
        }
        p3.data[x - 26] = '\0';
        //check if we got packet we are expecting
        int port = ntohs(server.sin_port);
        string ip = inet_ntoa(server.sin_addr);
        if(strncmp(p3.cmd, "MY_LIST", 7) == 0 && be64toh(p3.cmd_seq)==cmd_seq){
            cout << "Found "<<ip << " ("<<p3.data <<") with free space "<<be64toh(p3.param)<<"\n";
        }
        else{
            cout <<"[PCKG ERROR]  Skipping invalid package from "<<ip<<":"<<port<<".\n";
        }
    }

}


int main(int argc, const char *argv[]) {
    struct timeval s_timeout;
    s_timeout.tv_usec = 10;
    s_timeout.tv_sec = 0;
    uint16_t port;
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
    struct sockaddr_in remote_address;
    struct pollfd fds[N];


    //new connections on fds[0]
    int sock = socket(PF_INET, SOCK_DGRAM, 0); // creating IPv4 UDP socket
    if(sock < 0){
        syserr("socket");
    }

    //set timeout
    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&s_timeout, sizeof(s_timeout)) < 0) {
        error("setsockopt rcvtimeout\n");
        close(sock);
        exit(1);
    }
    //activate bcast
    int optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof optval) < 0)
        syserr("setsockopt broadcast");

/* Enable SO_REUSEADDR to allow multiple instances of this */
/* application to receive copies of the multicast datagrams. */
    int reuse = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
        syserr("secksockopt: reuse");
    }

    //binding port number with ip address
    memset((char *) &localSock, 0, sizeof(localSock));
    localSock.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    localSock.sin_port = htons(0); // listening on port PORT_NUM
    localSock.sin_family = AF_INET; // IPv4

    if(bind(sock, (struct sockaddr*)&localSock, sizeof(localSock)) < 0){
        syserr("bind");
    }


    /* ustawienie adresu i portu odbiorcy */

    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(port);
    if (inet_aton(addr.c_str(), &remote_address.sin_addr) == 0)
        syserr("inet_aton");
   /* if (connect(sock, (struct sockaddr *)&remote_address, sizeof remote_address) < 0)
        syserr("connect");*/

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
                perform_discover(sock, remote_address);
            }
            else if(a == "exit"){
                exit(0);
            }
            else if(a == "search"){
                //TODO search
                cout <<"performing search..\n";
                string s = "";
                perform_search(s, sock, remote_address);
            }
            else{
                cout << a << " is unrecognized command or requires parameter\n";
            }
        }
        else{
          string c;
          while(iss >> c){
            b+=" "+c;
          }
            if(a == "fetch"){
                //TODO fetch
                perform_fetch(b, sock, remote_address);
            }
            else if(a == "search"){
                //TODO search with argument
                perform_search(b, sock, remote_address);
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