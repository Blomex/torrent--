//
// Created by Beniamin on 18.05.2019.
//

#include <iostream>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
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
namespace fs = boost::filesystem;
using namespace boost::algorithm;
using std::string;
using std::cout;
using std::vector;
using std::cin;
using std::getline;
#define BSIZE         256
#define TTL_VALUE     4
#define REPEAT_COUNT  8
#define SLEEP_TIME    1
#define MAX_POOL 128

void on_timeout(int timeout){
    if(timeout> 300 || timeout <= 0){
        throw std::invalid_argument("Timeout value specified by -t or --TIMEOUT must be between 1 and 300");
    }
}


bool is_pattern_in_file(const fs::path &path, string &pattern){
        cout << "Checking if file "<<path.filename()<<" contains designated fragment\n";
        fs::ifstream ofs{path};
        fs::ifstream ofs2{path};
        unsigned long patternLength = pattern.length();
        char buffer[patternLength * 2];
        ofs2.read(buffer, patternLength);
        while (ofs) {
            ofs.read(buffer, patternLength * 2);
            string pom = string(buffer);
            if (pom.find(pattern) != string::npos) {
                return true;
            }
        }
        while(ofs2){
            ofs2.read(buffer, patternLength *2);
            if(string(buffer).find(pattern) != string::npos){
                return true;
            }
        }
        return false;
}
vector<fs::path> check_all_files_for_pattern(vector <fs::path> files, string &pattern){
   vector<fs::path> filenames;
    for(auto &f: files){
        if(is_pattern_in_file(f, pattern)){
            cout << "found!\n";
            filenames.emplace_back(f.filename());
        }
    }
    return filenames;
}

send_file_list_packet(int sock, struct sockaddr_in dest, string &s)

int main(int argc, const char *argv[]) {
    struct sockaddr_in src_addr;
    struct timeval timeout;
    timeout.tv_usec = 0;

    SIMPL_CMD simple_cmd;
    uint16_t port;
    uint64_t space;
    string addr, disc_folder;
    struct ip_mreq group;
    try {
        options_description desc{"Options"};
        desc.add_options()
            ("MCAST_ADDR,g", value<string>(&addr)->required(), "adress")
            ("CMD_PORT,p", value<uint16_t>(&port)->required(), "port")
            ("MAX_SPACE,b", value<uint64_t>(&space)->default_value(52428800), "max_space")
            ("SHRD_FLDR,f", value<string>(&disc_folder)->required(), "disc folder")
            ("TIMEOUT,t", value<time_t>(&(timeout.tv_sec))->default_value(5)->notifier(on_timeout), "timeout");
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
    //index files in folder
    std::vector<fs::path> Files;
    cout <<"Indexing files in "<<fs::directory_entry(disc_folder) << " ...\n";
    int spaceTaken = 0;
    try {
        for (auto &p: fs::directory_iterator(disc_folder)) {
            if (fs::is_regular(p)) {
                cout << "size of: " << p.path().filename() << " is " << fs::file_size(p) << "\n";
                spaceTaken += fs::file_size(p);
                Files.push_back(p.path());
            }
        }
    }
    catch(fs::filesystem_error &err){
        cout <<"Exception thrown: "<< err.what() << "\n";
        cout <<"Closing the server";
    }
    cout << "Indexing complete, total size: " << spaceTaken << "\n";
    cout << "Free size left: ";
    cout << space - spaceTaken << "\n";
    string str = "CMAKE_BINARY_DIR = /mnt/c/Users/Beniamin/CLionProjects/untitled21/cmake-build-debug\n";
   // vector<fs::path> nowy = check_all_files_for_pattern(Files, str);
    /*for(auto &a : nowy){
        cout << a;
    }*/
    /* zmienne i struktury opisujące gniazda */
    int sock, optval;
    struct sockaddr_in local_address;
    struct sockaddr_in remote_address;


    /* zmienne obsługujące komunikację */
    socklen_t len = sizeof(struct sockaddr_in);
    size_t length;
    time_t time_buffer;
    char buffer[BSIZE];

    /*inicjacja pooli*/
    struct pollfd fds[MAX_POOL];
    for(int i=0; i<MAX_POOL; i++){
        fds[i].fd = -1;
        //sprawdzamy czy nadeszly dane do odczytu
        fds[i].events = POLLIN;
        //narazie jeszcze funkcja poll() nie zostala wykonana
        fds[i].revents = 0;
    }

    /* otworzenie gniazda */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        syserr("socket");

    /*reuse addr*/
    {
        int reuse=1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
            perror("setting SO_REUSEADDR");
            close(sock);
            exit(1);
        }
    }


/* podpięcie się pod lokalny adres i port */
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *)&local_address, sizeof local_address) < 0) {
        syserr("bind");
    }

    /* podpięcie się do grupy rozsyłania (ang. multicast) */
    group.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(addr.c_str(), &group.imr_multiaddr) == 0)
        syserr("inet_aton");

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&group, sizeof group) < 0)
        syserr("setsockopt");


    /* uaktywnienie rozgłaszania (ang. broadcast) */
    optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof optval) < 0)
        syserr("setsockopt broadcast");

    /* ustawienie TTL dla datagramów rozsyłanych do grupy */
//    optval = TTL_VALUE;
//    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval, sizeof optval) < 0)
//        syserr("setsockopt multicast ttl");

//    /* zablokowanie rozsyłania grupowego do siebie */
//    optval = 0;
//    if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void*)&optval, sizeof optval) < 0)
//      syserr("setsockopt loop");

  fds[0].fd = sock;

  /*czytanie wszystkiego z socketu UDP*/
  bool xd = true;
  do {
      poll(fds, MAX_POOL, -1);
          if(fds[0].fd & POLLIN){
          CMPLX_CMD *abc;
          simple_cmd.cmd_seq = 6666;
          cout << "GG, pollin works! Poggers\n";
          memset(&src_addr, 0, sizeof(src_addr));
          ssize_t recv_len = recvfrom(sock, &simple_cmd, sizeof simple_cmd, 0, (struct sockaddr *) &src_addr, &len);

          cout << "say hi\n";
          int i = recv_len;
          printf("received : %d\n", i);
          if (strncmp(simple_cmd.cmd, "HELLO", 5) == 0) {
              cout << "WE GOT HELLO, WOAH\n";
              CMPLX_CMD complex;
              complex.cmd_seq = simple_cmd.cmd_seq;
              complex.param = htobe64((uint64_t)space-spaceTaken);
              strcpy(complex.data, inet_ntoa(group.imr_multiaddr));
              set_cmd(complex.cmd, "GOOD_DAY");
              int send = 26 + strlen(inet_ntoa(group.imr_multiaddr));

              if(sendto(sock, &complex, send, 0, (struct sockaddr *)&src_addr, (socklen_t)sizeof (src_addr)) != send) {
                  syserr("sendto");
              }
          }
          else if(strncmp(simple_cmd.cmd, "GET", 3) == 0){
              abc = (CMPLX_CMD *) &simple_cmd;

          }
          else if(strncmp(simple_cmd.cmd, "LIST", 4) == 0){
              cout << "Search..";
              simple_cmd.data[recv_len-18] = '\0';
              send_file_list_packet(sock, remote_address, simple_cmd.data);
          }
          else{
          }
      }
      for(int i=1; i < MAX_POOL; i++){

          if(fds[1].fd & POLLIN){

          }
      }
  }while(xd);



}