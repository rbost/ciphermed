#pragma once


// abstract classes: we want common calls to private comparators for comparisons over encrypted data

// party A has the public parameters and gets the encrypted result
class Comparison_protocol_A {
public:
    virtual void set_value(const mpz_class &x) {}
    virtual mpz_class output() const { }
    virtual GM gm() const { }
};

// party B has the secret parameters
class Comparison_protocol_B {
public:
    virtual void set_value(const mpz_class &x) {}
    virtual GM_priv gm() const { };
};