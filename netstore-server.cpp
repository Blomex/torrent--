//
// Created by Beniamin on 18.05.2019.
//

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
#include <arpa/inet.h>
using namespace boost::program_options;
using namespace boost::algorithm;
using std::string;
using std::cout;
using std::cin;
using std::getline;
#define BSIZE         256
#define TTL_VALUE     4
#define REPEAT_COUNT  8
#define SLEEP_TIME    1

void on_timeout(int timeout){
    if(timeout> 300 || timeout <= 0){
        throw std::invalid_argument("Timeout value specified by -t or --TIMEOUT must be between 1 and 300");
    }
}

int main(int argc, const char *argv[]) {
    uint16_t port;
    int timeout;
    uint64_t space;
    string addr, disc_folder;
    struct ip_mreq group;
    try {
        options_description desc{"Options"};
        desc.add_options()
            ("MCAST_ADDR,g", value<string>(&addr)->required(), "adress")
            ("CMD_PORT,p", value<uint16_t>(&port)->required(), "port")
            ("MAX_SPACE,b", value<uint64_t>(&space), "max_space")
            ("SHRD_FLDR,f", value<string>(&disc_folder)->required(), "disc folder")
            ("TIMEOUT,t", value<int>(&timeout)->default_value(5)->notifier(on_timeout), "timeout");
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);
    }
    catch (const error &ex) {
        std::cerr << ex.what() << '\n';
        exit(0);
    }
    catch (std::invalid_argument &ex) {
        std::cerr << ex.what() << '\n';
        exit(0);
    }


    /* zmienne i struktury opisujące gniazda */
    int sock, optval;
//  struct sockaddr_in local_address;
    struct sockaddr_in remote_address;

    /* zmienne obsługujące komunikację */
    size_t length;
    time_t time_buffer;
    int i;
    char buffer[BSIZE];


    /* otworzenie gniazda */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        syserr("socket");

    /* uaktywnienie rozgłaszania (ang. broadcast) */
    optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof optval) < 0)
        syserr("setsockopt broadcast");

    /* ustawienie TTL dla datagramów rozsyłanych do grupy */
    optval = TTL_VALUE;
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval, sizeof optval) < 0)
        syserr("setsockopt multicast ttl");

    /* zablokowanie rozsyłania grupowego do siebie */
    /*
    optval = 0;
    if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void*)&optval, sizeof optval) < 0)
      syserr("setsockopt loop");
    */

    /* podpięcie się pod lokalny adres i port */
//  local_address.sin_family = AF_INET;
//  local_address.sin_addr.s_addr = htonl(INADDR_ANY);
//  local_address.sin_port = htons(0);
//  if (bind(sock, (struct sockaddr *)&local_address, sizeof local_address) < 0)
//    syserr("bind");


    /* ustawienie adresu i portu odbiorcy */
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(port);
    if (inet_aton(addr.c_str(), &remote_address.sin_addr) == 0)
        syserr("inet_aton");
    if (connect(sock, (struct sockaddr *)&remote_address, sizeof remote_address) < 0)
        syserr("connect");

    /* radosne rozgłaszanie czasu */
    for (i = 0; i < REPEAT_COUNT; ++i) {
        time(&time_buffer);
        strncpy(buffer, ctime(&time_buffer), BSIZE);
        length = strnlen(buffer, BSIZE);
        if (write(sock, buffer, length) != length)
            syserr("write");
        sleep(SLEEP_TIME);
    }

    /* koniec */
    close(sock);
    exit(EXIT_SUCCESS);



}