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
#include <atomic>
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
using std::string;
using std::getline;
#define BSIZE         256
#define TTL_VALUE     4
#define REPEAT_COUNT  8
#define SLEEP_TIME    1
#define MAX_POOL 128
uint64_t size;
struct timeval timeout;
void on_timeout(int timeout){
    if(timeout> 300 || timeout <= 0){
        throw std::invalid_argument("Timeout value specified by -t or --TIMEOUT must be between 1 and 300");
    }
}
void set_server_options(string &addr, uint16_t &port, uint64_t &space, string &disc_folder, struct timeval &timeout, int argc, const char *argv[]){
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
}

void send_file_list_packet(int sock, struct sockaddr_in dest, SIMPL_CMD &received, vector<fs::path> files){
    SIMPL_CMD packet;
    set_cmd(packet.cmd, "MY_LIST");
    packet.cmd_seq = received.cmd_seq;
    int data_counter = 0;
    string s = string(received.data);
    int delimiter_counter = -1;
    vector<string> names;
    for(auto &f: files) {
        if (f.filename().string().find(s) != string::npos) {
            if (data_counter + delimiter_counter + f.filename().string().length() > MAX_DATA_SIZE) {
                strcpy(packet.data, boost::join(names, "\n").c_str());
                if (sendto(sock,
                           &packet,
                           (size_t) 18 + data_counter + delimiter_counter,
                           0,
                           (struct sockaddr *) &dest,
                           sizeof(dest)) != data_counter + 18) {
                    syserr("sendto");
                }
                data_counter = 0;
                delimiter_counter = -1;
                names = vector<string>();
            }
            names.push_back(f.filename().string());
            delimiter_counter++;
            data_counter += f.filename().string().length();
        }
        strcpy(packet.data, boost::join(names, "\n").c_str());
        if (data_counter > 0) {
            if (sendto(sock,
                       &packet,
                       (size_t) 18 + data_counter + delimiter_counter,
                       0,
                       (struct sockaddr *) &dest,
                       sizeof(dest)) != data_counter + 18) {
                syserr("sendto");
            }
        }
    }

}

uint64_t index_files(vector<fs::path> &Files, string &disc_folder, uint64_t space){
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
    size = space - spaceTaken;
    cout << size << "\n";
    return size;
}

int create_new_tcp_socket(uint16_t &port){
    int sock;
    struct sockaddr_in serveraddr;
    sock = socket(AF_INET, SOCK_STREAM, 0);//TCP
    if(sock<0){
        syserr("socket");
    }
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(0);
    //timeout for accept
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
        error("setsockopt rcvtimeout\n");
        shutdown(sock, SHUT_WR);
        exit(1);
    }

    if (bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        syserr("bind");
    if (listen(sock, 1) < 0)
        syserr("listen");
    port = ntohs(serveraddr.sin_port);
    return sock;
}
void send_can_add(string &file, uint16_t port, int sock, struct sockaddr_in client, int main_sock){
    CMPLX_CMD complex;
    set_cmd(complex.cmd, "CAN_ADD");
    complex.param = htobe64(uint64_t(port));
    strncpy(complex.data, file.c_str(), file.length());
    int size_to_send = 26 + file.length();
}
void receive_file(string &file, int main_sock, struct sockaddr_in client){
    struct sockaddr_in private_client;
    socklen_t client_address_len = sizeof(private_client);
    uint16_t port;
    int sock = create_new_tcp_socket(port);
    //send "can add"
    send_can_add(file, port, sock, client, main_sock);
}
void send_connect_me(string &file, uint16_t port, int sock, struct sockaddr_in client, int main_sock){
    CMPLX_CMD complex;
    set_cmd(complex.cmd, "CONNECT_ME");
    complex.param = htobe64(uint64_t(port));
    strncpy(complex.data, file.c_str(), file.length());
    int size_to_send = 26 + file.length();
    if(sendto(main_sock, (char*)&complex, size_to_send,0 , (struct sockaddr*)&client, sizeof(client)) != size_to_send){
        syserr("partial sendto");
    }

}
void send_file(string &file, int main_sock, struct sockaddr_in client){
    uint16_t port;
    struct sockaddr_in private_client;
    socklen_t client_address_len = sizeof(private_client);
    int sock = create_new_tcp_socket(port);
    send_connect_me(file, port, sock, client, main_sock);
    //now wait for accept for timeout seconds.
    int msg_sock = accept(sock, (struct sockaddr*)&private_client,  &client_address_len);
    if(msg_sock<0){
        perror("accept");
        close(msg_sock);
        close(sock);
        return;
    }
    //we accepted so we can send now
    //send file
    fs::path filePath{file};
    std::ifstream stream(filePath.c_str(), std::ios::binary);
    uint64_t size = filePath.size();
    std::vector<char> buffer(size);
    if(stream.read(buffer.data(), size)){
        if(write(msg_sock, &buffer, size)!= size){
            syserr("partial/failed write");
        }
    }
    close(msg_sock);

}

int main(int argc, const char *argv[]) {
    struct sockaddr_in src_addr;
    timeout.tv_usec = 0;

    SIMPL_CMD simple_cmd;
    uint16_t port;
    uint64_t space;
    string addr, disc_folder;
    struct ip_mreq group;
    set_server_options(addr, port, space, disc_folder, timeout, argc, argv);
    //index files in folder
    std::vector<fs::path> Files;
    int size = index_files(Files, disc_folder, space);
    /* zmienne i struktury opisujące gniazda */
    int sock, optval;
    struct sockaddr_in local_address;
    struct sockaddr_in remote_address;


    /* zmienne obsługujące komunikację */
    socklen_t len = sizeof(struct sockaddr_in);
    size_t length;
    time_t time_buffer;
    char buffer[BSIZE];

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
//    /* zablokowanie rozsyłania grupowego do siebie */
//    optval = 0;
//    if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void*)&optval, sizeof optval) < 0)
//      syserr("setsockopt loop");


  /*czytanie wszystkiego z socketu UDP*/
  bool xd = true;
  do {
          CMPLX_CMD *abc;
          //TODO debug
          abc = (CMPLX_CMD *) &simple_cmd;
          simple_cmd.cmd_seq = 6666;
          cout << "GG, pollin works! Poggers\n";
          memset(&src_addr, 0, sizeof(src_addr));
          ssize_t recv_len = recvfrom(sock, &simple_cmd, sizeof simple_cmd, 0, (struct sockaddr *) &src_addr, &len);
          if (strncmp(simple_cmd.cmd, "HELLO", 5) == 0) {
              cout << "WE GOT HELLO, WOAH\n";
              CMPLX_CMD complex;
              complex.cmd_seq = simple_cmd.cmd_seq;
              complex.param = htobe64(size);
              strcpy(complex.data, inet_ntoa(group.imr_multiaddr));
              set_cmd(complex.cmd, "GOOD_DAY");
              int send = 26 + strlen(inet_ntoa(group.imr_multiaddr));

              if(sendto(sock, &complex, send, 0, (struct sockaddr *)&src_addr, (socklen_t)sizeof (src_addr)) != send) {
                  syserr("sendto");
              }
          }
          else if(strncmp(simple_cmd.cmd, "GET", 3) == 0){
              //TODO CONNECT ME
              simple_cmd.data[recv_len-18]='\0';
              string filename = simple_cmd.data;
              send_file(filename, sock, src_addr);
          }
          else if(strncmp(simple_cmd.cmd, "LIST", 4) == 0){
              cout << "Search..\n";
              simple_cmd.data[recv_len-18] = '\0';
              send_file_list_packet(sock, src_addr, simple_cmd, Files);
          }
          else if(strncmp(simple_cmd.cmd, "ADD", 10)){
              cout << "Add..\n";
              abc->data[recv_len-26] = '\0';
              string fname (abc->data);
              uint64_t file_size = be64toh(abc->param);
              if (strcmp(abc->data, "")==0 || fname.find('/')!= string::npos || file_size > size){
                  //TODO NO_WAY
                  SIMPL_CMD answer;
                  set_cmd(answer.cmd, "NO_WAY");
                  answer.cmd_seq = abc->cmd_seq;
                  strcpy(answer.data, abc->data);
                  int size = strlen(answer.data) + 18;
                  if(sendto(sock, &answer, size, 0, (struct sockaddr*)&src_addr, sizeof src_addr)){
                      syserr("sendto");
                  }
              }
              else{
                  receive_file(fname, sock, src_addr);
              }
          }
          else if(strncmp(simple_cmd.cmd, "DEL", 10) == 0 ){
              cout << "Delete..\n";
              simple_cmd.data[recv_len-18] = '\0';
              //delete file
              for (auto &f: Files){
                  if(strcmp(f.filename().c_str(), simple_cmd.data) == 0){
                      try {
                          fs::remove(f);
                          break;
                      }
                      catch(fs::filesystem_error &err){
                          cout << err.what();
                      }
                  }
              }
          }
          else{
          }
  }while(xd);



}
