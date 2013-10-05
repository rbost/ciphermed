#pragma once

#include <cstddef>
#include <vector>

#include <tree/m_variate_poly.hh>

using namespace std;

template <typename T> class Tree
{
public:
    virtual inline bool isLeaf() const = 0;
    virtual const T& decision(const vector<bool> &b_table) const = 0;
    virtual Multivariate_poly<T> to_polynomial() const = 0;
};

template <typename T> class Leaf : public Tree<T>
{
    T value_;
    
public:
    Leaf(T v) : value_(v) {}
    
    inline const T& value() const { return value_; }
    inline bool isLeaf() const { return true; }
    inline const T& decision(const vector<bool> &b_table) const { return value_; }
    
    Multivariate_poly<T> to_polynomial() const
    {
        return Multivariate_poly<T>(Term<T>(value_));
    }
};


template <typename T> class Node : public Tree<T>
{
    size_t index_;
    Tree<T> *left_, *right_;
    
public:
    Node(size_t i, Tree<T> *l, Tree<T> *r)
    : index_(i), left_(l), right_(r)
    {}
    
    ~Node()
    {
        delete left_;
        delete right_;
    }

    inline bool isLeaf() const { return false; }

    inline Tree<T>* leftChild() const { return left_; }
    inline Tree<T>* rightChild() const { return right_; }
    
    const T& decision(const vector<bool> &b_table) const
    {
        if (b_table[index_]) {
            return left_->decision(b_table);
        }else{
            return right_->decision(b_table);
        }
    }
    
    Multivariate_poly<T> to_polynomial() const
    {
        Multivariate_poly<T> p_l = left_->to_polynomial();
        Multivariate_poly<T> p_r = right_->to_polynomial();
        
        Term<T> b (1,{index_});
        return p_r + b*(p_l -p_r);
    }

};