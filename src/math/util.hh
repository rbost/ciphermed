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
