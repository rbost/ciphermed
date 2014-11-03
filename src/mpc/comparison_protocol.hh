#pragma once


// abstract classes: we want common calls to private comparators for comparisons over encrypted data

// party A has the public parameters and gets the encrypted result
class Comparison_protocol_A {
public:
    virtual void set_value(const mpz_class &x) = 0;
    virtual size_t bit_length() const { return 0; }
    virtual void set_bit_length(size_t l) = 0;
    
    virtual mpz_class output() const = 0;
    virtual GM gm() const = 0;
};

// party B has the secret parameters
class Comparison_protocol_B {
public:
    virtual void set_value(const mpz_class &x) = 0;
    virtual size_t bit_length() const { return 0; }
    virtual void set_bit_length(size_t l) = 0;

    virtual GM_priv gm() const = 0;
};

// dynamicaly call the right test function
void runProtocol(Comparison_protocol_A *party_a, Comparison_protocol_B *party_b, gmp_randstate_t state);


typedef enum
{
    LSIC_PROTOCOL = 0,
    DGK_PROTOCOL = 1,
    GC_PROTOCOL = 2
}COMPARISON_PROTOCOL;