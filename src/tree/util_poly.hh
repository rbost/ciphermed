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

#include <tree/m_variate_poly.hh>
#include <vector>
using namespace std;

// lexicographic order
template <typename T> int compareVars(const Term<T> &t1, const Term<T> &t2)
{
    size_t i = 0;
    while (i < t1.variables().size()) {
        if (i == t2.variables().size() || t1.variables()[i]<t2.variables()[i]) {
            return -1;
        }else if(t1.variables()[i]>t2.variables()[i]){
            return 1;
        }
        i++;
    }
    
    if (i < t2.variables().size()) {
        return 1;
    }
    return 0;
}

// regroup redundant terms using a divide & conquer algorithm very similar to merge sort
template <typename T> vector<Term <T> > mergeRegroup(const vector<Term <T> > &p)
{
    if (p.size() < 2) {
        return p;
    }
    
    size_t const half_size = p.size() / 2;
    vector<Term <T> > split_lo(p.begin(), p.begin() + half_size);
    vector<Term <T> > split_hi(p.begin() + half_size, p.end());
    
    split_lo = mergeRegroup(split_lo);
    split_hi = mergeRegroup(split_hi);
    
    vector<Term <T> > res;
    
    size_t i_lo = 0, i_hi = 0;
    
    while (i_lo < split_lo.size() && i_hi < split_hi.size()) {
        int c = compareVars(split_lo[i_lo],split_hi[i_hi]);
        
        if (c == 0) {
            // terms have the same variables, add them
            Term<T> t(split_lo[i_lo].coefficient() + split_hi[i_hi].coefficient(),split_lo[i_lo].variables());
            res.push_back(t);
            i_lo++; i_hi++;
        }else if(c < 0){
            res.push_back(split_lo[i_lo]);
            i_lo++;
        }else{ // c > 0
            res.push_back(split_hi[i_hi]);
            i_hi++;
        }
    }
    
    for (; i_lo < split_lo.size(); i_lo++) {
        res.push_back(split_lo[i_lo]);
    }
    for (; i_hi < split_hi.size(); i_hi++) {
        res.push_back(split_hi[i_hi]);
    }
    
    return res;
}

template <typename T> inline Multivariate_poly<T> mergeRegroup(const Multivariate_poly<T> &p)
{
    return Multivariate_poly<T>(mergeRegroup(p.terms()));
}