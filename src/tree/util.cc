#include <vector>
#include <cstddef>

#include <tree/util.hh>

using namespace std;

vector<long> bitDecomp(long x, size_t n)
{
    vector<long> bits(n);
    for (size_t i = 0; i < n ; i++) {
        bits[i] = x & 1;
        x>>=1;
    }
    
    return bits;
}

long bitDecomp_inv(const vector<long> &bits)
{
    long x = 0;
    size_t n = bits.size();
    for (size_t i = 1; i <= n ; i++) {
        x <<= 1;
        x += bits[n-i];
    }
    return x;
}