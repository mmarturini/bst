# Binary search tree
The goal of the project was to implement a template binary search tree (BST) in C++.

## Compile and run the code

To compile the source file "bst.cpp", type "make" from the command line in this directory.
Therefore, in order to launch the program, type "./bst.x" from command line.

The first part of the source file is the implementation of the bst data structure, hence the node structure, the member functions and the implementation of the forward iterator. In the main function, by defualt a BST will be initialized and printed; this BST has the same key values as the ones of the image provided on the assignment of the project (the values associated to the keys are chosen at random). Then there are multiple commented lines that can be uncommented to test the various member functions of the BST class.
The various lines have been tested and run with valgrind, and no memory leaks had been found.


## Report

To design a solution I started thinking about what the components of a binary search tree are, and how you can traverse the key values in ascending order: I therefore started by implementing the structure of each node and the forward iterator to traverse them.
The node is composed of the left and right unique pointers, the pair values and a raw pointer to the parent node; the custom constructors are implemented to be compatible with the insert function in case of an empty tree or not.

The iterator is such that when dereferenced it returns the pair values, and when the ++ operator is called, it exploits the parent, left and right pointers of each node to traverse the tree in the right order. I then employed the iterator and the nodes structure to implement the member functions required.

### Member functions
#### Insert
Insert a new node in the tree, and returns an iterator pointing to the newly inserted node and a boolean value equal to true if the key value was not already present, hence the insertion was successful. There are two variants of the function, one that takes as argument an rvalue, and one an lvalue; both refer to a single function implemented with the forwarding reference. This function calls 2 different types of node constructors depending if the tree is empty or not. 

#### Emplace
It exploits the variadic templates and the insert function to insert a new node given a pair istantiated by passing not one but two arguments (the two members of the pair).

#### Clear
It clears the content of the tree by resetting the root pointer, a unique pointer that points to the root of the tree.

#### Begin and end
"Begin" returns an iterator to the leftmost node of the tree, while "end" returns an iterator that points to nullptr. The cbegin and cend versions are also implemented. They are used to traverse the nodes of the tree with the range-based for loop.

#### Find
Given a key value, it will look for that key in the tree by passing through all nodes.

#### Balance
Balances the tree by copying the pair values in a std::vector in ascending order; then, after clearing the tree, it calls a recursive function named "recursive_insert" that insert the midpoint pair of the vector, then consider the left and right part, and insert the midpoint of the two parts, and so on until all pairs are inserted.

#### Operator []
Returns a reference to the value that is mapped to a key equivalent to the one passed as argument, performing an insertion if such key does not already exist.

#### Put-to operator
The overload of the put-to operator print the pair values of all the nodes present in the tree, in ascending order according to the key values.




