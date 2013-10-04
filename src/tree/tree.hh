#include <cstddef>
#include <vector>


using namespace std;

template <typename T> class Tree
{
public:
    virtual inline bool isLeaf() const = 0;
    virtual const T& decision(const vector<bool> &b_table) const = 0;
};

template <typename T> class Leaf : public Tree<T>
{
    T value_;
    
public:
    Leaf(T v) : value_(v) {}
    
    inline const T& value() const { return value_; }
    inline bool isLeaf() const { return true; }
    inline const T& decision(const vector<bool> &b_table) const { return value_; }
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
};