#pragma once

#include <vector>
#include <iostream>
#include <NTL/ZZX.h>
#include <EncryptedArray.h>

using namespace std;

template <typename T>
class Term {
    T coeff_;
    vector<size_t> variables_;
    
public:
    Term() : coeff_(0),variables_(0) {};
    Term(const T &c) : coeff_(c), variables_(0) {}
    Term(const T &c, const vector<size_t> vars)
    : coeff_(c), variables_(vars) {}
    
    const T& coefficient () const { return coeff_; }
    const vector<size_t>& variables () const { return variables_; }
    
    Term<T> multiplyBy(const T &v) const
    {
        return Term<T>(v*coeff_, variables_);
    }
    
    Term<T> multiplyBy(const Term<T> &term) const
    {
        vector<size_t> joint_v;
        joint_v.reserve(variables_.size() + term.variables().size());
        
        joint_v.insert(joint_v.end(),variables_.begin(),variables_.end());
        joint_v.insert(joint_v.end(),term.variables().begin(),term.variables().end());

        return Term<T>(coeff_*term.coefficient(),joint_v);
    }
    
    void scaleBy(const T &v) { coeff_ *= v; }
    void scaleBy(const Term<T> &term)
    {
        coeff_ *= term.coefficient();
        variables_.insert(variables_.end(),term.variables().begin(),term.variables().end());
    }
    
    Term<T> operator*(const Term<T> &right) const
    {
        return multiplyBy(right);
    }
    Term<T> operator*(const T &c) const
    {
        return multiplyBy(c);
    }
    
    Term<T> operator-() const
    {
        return Term<T>(-coeff_,variables_);
    }
    
    void operator*=(const Term<T> &right)
    {
        scaleBy(right);
    }

    void operator*=(const T &c)
    {
        scaleBy(c);
    }
};

template <typename T>
Term<T> operator*(const T &c, const Term<T> &t)
{
    return t.multiplyBy(c);
}

template <typename T> inline ostream& operator<<(ostream &out, const Term<T> & t)
{
    out << "[" << t.coefficient() << ", ";
    out << "{" ;
    for(size_t i = 0; i < t.variables().size(); i++)
    {
        if(i>0) out << ", ";
        
        out << (t.variables())[i];
    }
    out << "} ]";
    
    return out;
}

template <typename T, typename U = T, typename V = U> V evalTerm(const Term<T> &term, const vector<U> &vals)
{
    if (term.variables().size() == 0) {
        return term.coefficient();
    }
    V v = vals[term.variables()[0]];
    
    
    for (size_t i = 1; i < term.variables().size(); i++) {
        v *= vals[term.variables()[i]];
    }
    
    v*= term.coefficient();
    
    return v;
}

template <typename T, typename U = T, typename V = U>
vector<V> evalTerm(const Term< vector<T> > &term, const vector<U> &vals)
{
    if (term.variables().size() == 0) {
        return vector<V>(term.coefficient());
    }
    V v = vals[term.variables()[0]];
    
    
    for (size_t i = 1; i < term.variables().size(); i++) {
        v *= vals[term.variables()[i]];
    }
        
    vector<V> res(term.coefficient().size());
    for (size_t j = 0; j < term.coefficient().size(); j++) {
        res[j] = term.coefficient()[j] * v;
    }

    return res;

}

template <typename T>
class Multivariate_poly {
    vector<Term <T> > terms_;
    
public:
    Multivariate_poly() : terms_(0) {}
    Multivariate_poly(const vector<Term <T> > &t) : terms_(t) {}
    Multivariate_poly(const Term <T> &t) : terms_({t}) {}
    
    const vector< Term <T> >& terms() const { return terms_; }
        
    void operator+=(const Term<T> &t)
    {
        terms_.insert(terms_.end(),t);
    }

    void operator+=(const Multivariate_poly<T> &p)
    {
        terms_.insert(terms_.end(),p.terms().begin(),p.terms().end());
    }
    
    void operator*=(const Term<T> &t)
    {
        for(size_t i = 0; i < terms_.size(); i++)
        {
            terms_[i] *= t;
        }
    }
    
    void operator*=(const Multivariate_poly<T> &p)
    {
        terms_ = (*this * p).terms();
    }
};

template <typename T>
Multivariate_poly<T> operator+(const Term<T> &t1, const Term<T> &t2)
{
    return Multivariate_poly<T>({t1,t2});
}

template <typename T>
Multivariate_poly<T> operator+(const Term<T> &t1, const Multivariate_poly<T> &p2)
{
    vector< Term<T> >terms(p2.terms());
    terms.insert(terms.begin(),t1);
    return Multivariate_poly<T>(terms);
}

template <typename T>
Multivariate_poly<T> operator+(const Multivariate_poly<T> &p1, const Term<T> &t2)
{
    vector< Term<T> >terms(p1.terms());
    terms.insert(terms.end(),t2);
    return Multivariate_poly<T>(terms);
}

template <typename T>
Multivariate_poly<T> operator+(const Multivariate_poly<T> &p1, const Multivariate_poly<T> &p2)
{
    vector< Term<T> >terms(p1.terms());
    terms.reserve(p1.terms().size() + p2.terms().size());
    
    terms.insert(terms.end(),p2.terms().begin(),p2.terms().end());
    
    return Multivariate_poly<T>(terms);
}

template <typename T>
Multivariate_poly<T> operator-(const Multivariate_poly<T> &p)
{
    vector< Term<T> > terms(p.terms().size());
    
    for(size_t i = 0; i < p.terms().size(); i ++)
    {
        terms[i] = -(p.terms())[i];
    }
    
    return Multivariate_poly<T>(terms);
}

template <typename T>
Multivariate_poly<T> operator-(const Multivariate_poly<T> &p1, const Multivariate_poly<T> &p2)
{
    return p1 + (-p2);
}

template <typename T>
Multivariate_poly<T> operator*(const Multivariate_poly<T> &p1, const Term<T> &t2)
{

    vector< Term<T> > terms(p1.terms().size());

    for(size_t i = 0; i < p1.terms().size(); i ++)
    {
        terms[i] = (p1.terms())[i]*t2;
    }

    return Multivariate_poly<T>(terms);
}

template <typename T>
Multivariate_poly<T> operator*(const Term<T> &t1, const Multivariate_poly<T> &p2)
{
    return operator*(p2,t1);
}

template <typename T>
Multivariate_poly<T> operator*(const Multivariate_poly<T> &p1, const Multivariate_poly<T> &p2)
{
    
    if(p2.terms().size() == 0){
        return Multivariate_poly<T>();
    }
    
    Multivariate_poly<T> p = p1*(p2.terms())[0];

    for(size_t i = 1; i < p2.terms().size(); i ++){
        p += p1*(p2.terms())[i];
    }
    return p;
}

template <typename T> inline ostream& operator<<(ostream &out, const Multivariate_poly<T> & p)
{
    if(p.terms().size() == 0) out << "0";
    
    for(size_t i = 0; i < p.terms().size(); i++)
    {
        if(i>0) out << " + ";
        
        out << (p.terms())[i];
    }
    
    return out;
}

template <typename T, typename U = T, typename V = U> V evalPoly(const Multivariate_poly<T> &p, const vector<U> &vals)
{
    V v = 0;
    
    for (size_t i = 0; i < p.terms().size(); i++) {
        v += evalTerm<T,U,V>((p.terms())[i],vals);
    }
    
    return v;
}


/*
 * Instantiation for polynomial evaluated with FHE
 */
Ctxt evalTerm_FHE(const Term< vector<long> > &term, const vector<Ctxt> &vals, const EncryptedArray &ea);
Ctxt evalPoly_FHE(const Multivariate_poly< vector<long> > &poly, const vector<Ctxt> &vals, const EncryptedArray &ea);