#pragma once

#include <vector>
#include <cstddef>

using namespace std;

// takes the n first bits of x and put them in a vector 
vector<long> bitDecomp(long x, size_t n);
long bitDecomp_inv(const std::vector<long> &bits);

// overload +,-,* operators for the vector class

template <typename T>
vector<T> operator+(const vector<T>& v1, const vector<T>& v2) {
    assert(v1.size() == v2.size());
    
    size_t s = v1.size();
    
    vector<T> res(s);
    for(size_t i = 0; i < s; i++){
        res[i] = v1[i] + v2[i];
    }
    
    return res;
}
template <typename T>
vector<T> operator*(const vector<T>& v1, const vector<T>& v2) {
    assert(v1.size() == v2.size());
    
    size_t s = v1.size();
    
    vector<T> res(s);
    for(size_t i = 0; i < s; i++){
        res[i] = v1[i] * v2[i];
    }
    
    return res;
}

template <typename T>
vector<T> operator-(const vector<T>& v) {    
    size_t s = v.size();
    
    vector<T> res(s);
    for(size_t i = 0; i < s; i++){
        res[i] = -v[i];
    }
    
    return res;
}