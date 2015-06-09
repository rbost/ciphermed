/*
 * Copyright 2013-2015 Raphael Bost
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

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