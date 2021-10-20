#include "logger.h"

#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstddef>
#include <cstdint>

std::string CRton(std::string str){
    std::string new_str(str);
    size_t pos = 0;
    while (true) {
        pos = new_str.find('\n', pos+1);
        if (pos != std::string::npos){
            new_str.replace(pos, 1, "\\n");
        } else {
            break;
        }
    }
    return new_str;
}


std::string now_str(){
    std::chrono::system_clock::time_point today = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t ( today );
    std::string now(ctime(&tt));
    now.erase(now.find("\n"));
    return now;
}
