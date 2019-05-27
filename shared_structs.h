//
// Created by Beniamin on 04.05.2019.
//

#ifndef PROJECT_SHARED_STRUCTS_H
#define PROJECT_SHARED_STRUCTS_H
#include <cstdint>
#include <string>
#define MAX_DATA_SIZE 65489
typedef struct __attribute__((__packed__)){
    char cmd[10];
    uint64_t cmd_seq;
    char data[MAX_DATA_SIZE];
}SIMPL_CMD;

typedef struct __attribute__((__packed__)){
    char cmd[10];
    uint64_t cmd_seq;
    uint64_t param;
    char data[MAX_DATA_SIZE - sizeof(param)];
}CMPLX_CMD;

typedef struct{
    bool isSuccessful;
    uint64_t size;
    std::string filename;
}remote_file;

typedef struct{
    bool isSuccessful;
    std::string message;
}promise_message;

void set_cmd(char cmd[10], const std::string &message){
    unsigned long n = message.length();
    for(ulong i =0; i < n; i++){
        cmd[i] = message[i];
    }
    for(ulong i = n; i < 10; i++){
        cmd[i] = '\0';
    }
}


#endif //PROJECT_SHARED_STRUCTS_H
