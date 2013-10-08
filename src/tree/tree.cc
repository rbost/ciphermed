#include <tree/tree.hh>

Tree<long>* balancedBinaryTree_aux(size_t n_leaves, size_t index, queue<size_t> &v_indices)
{
    assert(n_leaves > 0);
    if (n_leaves == 1) {
        return new Leaf<long>(index);
    }
    
    Tree <long> *left, *right;
    
    size_t n_leaves_left, n_leaves_right;
    n_leaves_right = n_leaves/2;
    n_leaves_left = n_leaves - n_leaves_right;
    
    left =  balancedBinaryTree_aux(n_leaves_left, index,v_indices);
    right = balancedBinaryTree_aux(n_leaves_right, index+n_leaves_left,v_indices);
    
    Tree<long> *res = new Node<long>(v_indices.front(),left,right);
    v_indices.pop();
    return res;

}

Tree<long>* balancedBinaryTree(size_t n_leaves)
{
    assert(n_leaves > 0);

    if (n_leaves == 1) {
        return new Leaf<long>(0);
    }
    
    queue<size_t> v_indices;
    for (size_t i = 0; i < n_leaves-1; i++) {
        v_indices.push(i);
    }
    
    return balancedBinaryTree_aux(n_leaves,0,v_indices);
}