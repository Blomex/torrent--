//
// Created by beniamin on 24.05.19.
//

//reading from file using fs
#include <boost/filesystem.hpp>
#include <iostream>
#include <fcntl.h>
namespace fs = boost::filesystem;



int main(){
int fd = open("CMakeCache.txt", O_RDONLY);
fs::path file("Makefile");
std::cout << fs::file_size(file) << "\n";
fs::path newFile("Makefile3");
fs::ifstream ifs(file, std::ifstream::binary);
fs::ofstream ofs(newFile, std::ifstream::binary);

char buffer[50];
int size = 50;
int bytes = 1;
while(bytes != 0){
    bytes = read(fd, buffer, size);
 //   ifs.read(buffer, size);
//    std::streamsize bytes = ifs.gcount();
    ofs.write(buffer, bytes);
  //  std::cout << bytes << "\n";
}
ofs.close();
//ifs.close();
close(fd);
}