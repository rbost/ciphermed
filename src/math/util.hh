/*
 * Copyright 2013-2015 Raluca Ada Popa
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

// compute op0 * op1- type T must have the following:
// 1. a one-argument constructor which creates a "zero" T with pre-allocated
//    size.
// 2. a size() method
// 3. operator[]
template <typename T>
T
naive_multiply(const T &op0, const T &op1)
{
    T res(op0.size() + op1.size() + 1);
    for (size_t i = 0; i < op0.size(); i++)
        for (size_t j = 0; j < op1.size(); j++)
            res[i + j] += (op0[i] * op1[j]);
    return res;
}

template <typename T, typename U>
U
naive_polyeval(const T &op0, const U &op1)
{
    U ret, op1v;
    if (op0.size() == 0)
        return ret;
    ret = op0[0];
    op1v = op1;
    for (size_t i = 1; i < op0.size(); i++, op1v *= op1)
        ret += op0[i] * op1v;
    return ret;
}

/* vim:set shiftwidth=4 ts=4 sts=4 et: */
