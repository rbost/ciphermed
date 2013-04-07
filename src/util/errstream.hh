#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

struct FHEError {
 public:
    FHEError(const std::string &m) : msg(m)
    {
    }
    std::string msg;
};

class fatal : public std::stringstream {
 public:
    ~fatal() __attribute__((noreturn)) {
        std::cerr << str() << std::endl;
        exit(-1);
    }
};

class fhe_err : public std::stringstream {
 public:
    ~fhe_err() {
        std::cerr << str() << std::endl;
        throw FHEError(str());
    }
};

class thrower : public std::stringstream {
 public:
    ~thrower() __attribute__((noreturn)) {
        throw std::runtime_error(str());
    }
};
