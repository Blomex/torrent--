//
// Created by Beniamin on 04.05.2019.
//

#ifndef PROJECT_SHARED_STRUCTS_H
#define PROJECT_SHARED_STRUCTS_H
#include <cstdint>
typedef struct{
    char cmd[10];
    uint64_t cmd_seq;
    char *data;
}SIMPL_CMD;

typedef struct{
    char cmd[10];
    uint64_t cmd_seq;
    uint64_t param;
    char *data;
}CMPLX_CMD;

#endif //PROJECT_SHARED_STRUCTS_H
