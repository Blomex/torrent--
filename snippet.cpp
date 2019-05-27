//
// Created by beniamin on 24.05.19.
//

//reading from file using fs
#include <boost/filesystem.hpp>
#include <iostream>
#include <fcntl.h>
namespace fs = boost::filesystem;
#include <string>
using std::string;


int main(){
int fd = open("CMakeCache.txt", O_RDONLY);
fs::path file("Project.cbp");
std::cout << fs::file_size("Makefile") << " xd \n";
std::cout << fs::file_size(file) << "\n";
string temp = "temp";
string Mfile3 = "Makefile71";
fs::path newFile(temp + "/" + Mfile3);
fs::ifstream ifs(file, std::ifstream::binary);
fs::ofstream ofs(newFile, std::ios::binary);

char buffer[500000];
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