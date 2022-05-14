#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory> 
#include <utility> 
#include <type_traits> //std::conditional
#include <vector>

#include "ap_error.h"



template <typename k_t, typename v_t, typename OP = std::less<k_t> > class bst {
  
  struct node {
    std::pair<const k_t, v_t> data;
    std::unique_ptr<node> left;
    std::unique_ptr<node> right;
    node* parent;  //pointer to the parent node
        
    explicit node(const std::pair<k_t,v_t> &x) : data{x}, parent{nullptr} {} //called by insert when empty tree
    
    explicit node(const std::pair<k_t,v_t> &&x) : data{std::move(x)}, parent{nullptr} {} //rvlaue
    
    node(const std::pair<k_t,v_t> &x, node *p) : data{x}, parent{p} {}  // called by insert when tree has already a root
    								          // the child will point to the parent   
    node(const std::pair<k_t,v_t> &&x, node *p) : data{std::move(x)}, parent{p} {}
  };
     
  // copy
  void __copy(const std::unique_ptr<node> &p) {
    if (p) {
      insert(p->data);
      __copy(p->left);
      __copy(p->right);
    }
    //when p == nullptr, will go out of scope
  }
  
public: 
  std::unique_ptr<node> root;
  OP op; 
  
  // Ctor and dtor
  bst() = default;
  ~bst() noexcept = default;
  
  //Move semantic
  bst(bst &&) noexcept = default;
  bst &operator=(bst &&) noexcept = default;
  
  //Copy semantic
  bst(const bst &x) {    
    __copy(x.root);
  }

  bst &operator=(const bst &x) {
    root.reset();
    auto tmp = x;           // copy ctor
    *this = std::move(tmp); // move assignment
    return *this;
  }
    
  // Declaring iterator
  template <typename O_k, typename O_v, bool Const> class _iterator;
  using iterator = _iterator<k_t,v_t,false>;         
  using const_iterator = _iterator<k_t,v_t,true>;
  
  // Begin and end
  iterator begin() noexcept {//starting from root, I got to the leftmost node 
    node *tmp = root.get();  //tmp: raw pointer that points to root
    while (tmp->left) {
      tmp = tmp->left.get();
    }
    return iterator{tmp};
  }
  
  const_iterator begin() const noexcept {
    node *tmp = root.get(); 
    while (tmp->left) {
      tmp = tmp->left.get(); 
    }
    return const_iterator{tmp};
  }
  
  const_iterator cbegin() const noexcept { return begin(); }
  
  iterator end() noexcept { return iterator{nullptr}; }
  const_iterator end() const noexcept { return const_iterator{nullptr}; }
  const_iterator cend() const noexcept { return const_iterator{nullptr}; }
  
  // INSERT 
  template <typename U>
  std::pair<iterator, bool> _insert(U &&x) {
    // if the bst has at least the root node, it enters the while
    node *current = root.get();
    while (current) {
      if (op(x.first, current->data.first)) { //if current does not have left and its key its greater than the inserting one
        if (!current->left) { // (current->left == nullptr)
          current->left.reset(new node{std::forward<U>(x), current});
          iterator it{current->left.get()};
          return std::pair<iterator, bool>{it,true};
        }
        else {
          current = current->left.get();
        }
      }
      else if (op(current->data.first, x.first)) { 
        if (!current->right) {
          current->right.reset(new node{std::forward<U>(x), current});
          iterator it{current->right.get()};
          return std::pair<iterator, bool>{it,true};
        }
        else {
          current = current->right.get();
        }
      }
      else { //the key is equal
        iterator it{current};
        return std::pair<iterator, bool>{it, false};
      }
    }
  
    //otherwise the tree is empty (root == nullptr)
    root.reset(new node{std::forward<U>(x)}); //this node ctor will call the one with just one arg (the pair)
    iterator it{root.get()};
    return std::pair<iterator, bool>{it,true};
  }
  
  std::pair<iterator, bool> insert(const std::pair<const k_t, v_t>& x) { 
    return _insert(x);
  }
  
  std::pair<iterator, bool> insert(std::pair<const k_t, v_t>&& x) { 
    return _insert(std::move(x));
  }
  
  
  // EMPLACE
  template< class... Types >
  std::pair<iterator,bool> emplace(Types&&... args) {
    return insert(std::pair<const k_t, v_t>{std::forward<Types>(args)...});
  }
    
    
  // CLEAR
  void clear() noexcept { root.reset(); }
  

  // BALANCE
  void recursive_insert(std::vector<std::pair<const k_t,v_t>>& vec, int start, int end) {

    if (start > end) {
      return;
    }
    
    if (start == end) {
      insert(vec[start]);
      return;
    }
    
    int m = (start + end) / 2;
    insert(vec[m]);
    
    recursive_insert(vec, start, m-1);
    recursive_insert(vec, m+1, end);
  }
  
  
   void balance() { 
    std::vector<std::pair<const k_t, v_t>> v;
     for (auto x : *this) {
       v.push_back(x);
     }

     clear();
     
     recursive_insert(v,0,v.size()-1);    
   }
      
      
   // FIND
   iterator find(const k_t& x) noexcept {
     for (iterator it{begin()}; it != end(); ++it) {
       if ( !op(it->first, x) && !op(x, it->first) ) {
         return it; 
       }
     }
     return end();
   }
   
   const_iterator find (const k_t& x) const noexcept {
     for (const_iterator it{begin()}; it != end(); ++it) {
       if ( !op((*it).first, x) && !op(x, (*it).first) ) {
         return it; 
       }
     }
     return end();
   }
   
   
   // SUBSCRIPTING OPERATOR
   v_t& operator[](const k_t& x) {
    iterator it{find(x)};
    if ( it.get_curr() ) {
      return it->second;
    }
    else {
      insert(std::pair<k_t,v_t>{x,{}});
      return find(x)->second;
    }
  }
  
  v_t& operator[](k_t&& x) {
    iterator it{find(x)};
    if ( it.get_curr() ) {
      return it->second;
    }
    else {
      insert(std::pair<k_t,v_t>{x,{}});
      return find(x)->second;
    }
  }
  

  friend
  std::ostream& operator<<(std::ostream& os, const bst& x) {
  
    try { //throw an error if you try to print an empty tree
      AP_ERROR(x.root.get()) <<"BST is empty" <<std::endl;
    } catch (const std::exception& e)  {
      std::cerr << e.what() << std::endl;
      os << " " <<std::endl;
      return os;
    }
   
    iterator it{x.root.get()};
    for (auto v : x) {
      os << "<"<<v.first <<","<<v.second <<">" <<std::endl;
    }
    return os;
  }
  
  
  // ERASE 
  bool two_child(iterator& it) const noexcept { //given an iterator, return true if it is pointing to a node with 2 children
    if ( it.get_curr()->left.get() && it.get_curr()->right.get() ) {
      return true;
    }
    else 
      return false;
  }
  
  bool left_child(iterator& it) const noexcept { //given an iterator, return true if it is pointing to a node with a left child
    if ( (it.get_curr()->left.get()) && !(it.get_curr()->right.get())  ) {
      return true;
    }
    else 
      return false;
  }
  
  bool right_child(iterator& it) const noexcept { //given an iterator, returns true if it is pointing to a node with a right child
    if ( (!it.get_curr()->left.get()) && (it.get_curr()->right.get())  ) {
      return true;
    }
    else 
      return false;
  }
  
  bool no_child(iterator& it) const noexcept { //given an iterator, returns true if it is pointing to a node with no children
    if ( (!it.get_curr()->left.get()) && !(it.get_curr()->right.get()) ) {
      return true;
    }
    else 
      return false;
  } 
  
  bool am_I_root(iterator& it) const noexcept{ //given an iterator, returns true if that iterator is pointing to the root
    if ( root.get() == it.get_curr() ) {
      return true;
    }
    else 
      return false;
  }
  
  bool am_I_right(iterator& it) const noexcept { //given an iterator, returns true if that iterator is pointing to a right child
    if ( op(it.get_curr()->parent->data.first, (*it).first) ) {
      return true;
    }
    else 
      return false;
  }
  
  bool am_I_left(iterator& it) const noexcept{ //given an iterator, returns true if that iterator is pointing to a left child
    if ( op((*it).first, it.get_curr()->parent->data.first) ) { 
    //dereferencing get_curr to access parent, the deref parent to access data
      return true;
    }
    else 
      return false;
  } 
  
  void erase(const k_t& x) {
    iterator it{find(x)}; 
    
    
    if ( it != end() ) {
      
      if ( am_I_root(it) ) {
        if ( two_child(it) ) {					//swap(it,next)
          try {							//erase(k)
            AP_ERROR( !two_child(it) ) <<"Cannot erase" <<std::endl;
          } catch (const std::exception& e)  {
            std::cerr << e.what() << std::endl;
            return;
          }
        }
        else if ( no_child(it) ) {
          root.reset(); 
        }
        else if ( left_child(it) ) {
          root.reset( it.get_curr()->left.release() ); 
        }
        else if ( right_child(it) ) {
          root.reset( it.get_curr()->right.release() ); 
        }
      }
      
      
      else if ( am_I_right(it) ) {
        if ( two_child(it) ) {
          try {
            AP_ERROR(!two_child(it)) <<"Cannot erase: node has 2 children" <<std::endl;
          } catch (const std::exception& e)  {
            std::cerr << e.what() << std::endl;
            return;
          }
        }
        else if ( no_child(it) ) {   
          it.get_curr()->parent->right.reset();
        }
        else if ( left_child(it) ) {
          it.get_curr()->left->parent = it.get_curr()->parent;
          it.get_curr()->parent->right.reset( it.get_curr()->left.release() );
        }
        else if ( right_child(it) ) {
          it.get_curr()->right->parent = it.get_curr()->parent;
          it.get_curr()->parent->right.reset( it.get_curr()->right.release() ); 
        }
      }
      
      
      else if ( am_I_left(it) ) {
        if ( two_child(it) ) {
          try {
            AP_ERROR(!two_child(it)) <<"Cannot erase" <<std::endl;
          } catch (const std::exception& e)  {
            std::cerr << e.what() << std::endl;
            return;
          }
        }
        else if ( no_child(it) ) { 
          it.get_curr()->parent->left.reset();                 
        }
        else if ( left_child(it) ) {
          it.get_curr()->left->parent = it.get_curr()->parent;
          it.get_curr()->parent->left.reset( it.get_curr()->left.release() ); 
        }
        else if ( right_child(it) ) {
          it.get_curr()->right->parent = it.get_curr()->parent;
          it.get_curr()->parent->left.reset( it.get_curr()->right.release() ); 
        }
      }
    }
    
    else { // if the iterator is pointing to end(), it means find(k) did not find the key
      std::cout <<"Key not present" <<std::endl;
    }
  }
  
};


template <typename k_t, typename v_t, typename OP>
template <typename O_k, typename O_v, bool Const> 
class bst<k_t,v_t,OP>::_iterator {

  using node = typename bst<k_t,v_t,OP>::node;
  node *current;
  
public:
  //when I dereference the iterator, I want the pair
  // if Const = true, then value_type is const pair (const iterator), otherwise is just pair
  using pair_type = std::pair<const O_k, O_v>;
  using value_type = typename std::conditional<Const, const pair_type, pair_type>::type; 
  using reference = value_type &;
  using pointer = value_type *;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag; 
  
  explicit _iterator(node* p): current{p} {} //if I pass a pointer to node to iterator it will initialize its only member current
  
  // * and -> operators
  reference operator*() const { return current->data; }
  pointer operator->() const { return &**this; }
  
  // ++ operator
  _iterator &operator++() {
    if (!current) { //If current is pointing to NULL
      return *this;
    }
    else if (current->right) { 	 //if the right pointer of current is not nullptr
      current = current->right.get();  // current is just a raw pointer, now it points to the right of the previous current
      while (current->left) {          // I keep going to the left until left points to nullptr 
        current = current->left.get();  
      }
    }
    else {
      node *tmp = current->parent;
      while ( tmp && current == tmp->right.get() ) { //while current has a parent, and he is the right child
        current = tmp;
        tmp = current->parent;
      }  					//at the end of the while, I'm a left child
      current = tmp;
    }
    return *this;
  }
  
  // post-increment
  _iterator operator++(int) {
    auto tmp{*this}; 
    ++(*this);  
    return tmp; 
  } 
  
  node* get_curr() {
    return current;
  } 
  
  friend bool operator==(const _iterator &a, const _iterator &b) {
    return a.current == b.current;
  }

  friend bool operator!=(const _iterator &a, const _iterator &b) { return !(a == b); }
};




int main () {

  // Testing insert and emplace
  bst<int,int> tree;
  
  tree.insert(std::pair<int,int> (8,1));
  tree.insert(std::pair<int,int> (10,2));
  tree.insert(std::pair<int,int> (14,3));
  tree.insert(std::pair<int,int> (13,4));
  tree.insert(std::pair<int,int> (3,5));
  tree.insert(std::pair<int,int> (6,6));
  tree.insert(std::pair<int,int> (7,7));
  tree.insert(std::pair<int,int> (4,8));
  tree.insert(std::pair<int,int> (1,8));
  
  //std::pair<int,int> paio(15,9);
  //tree.insert(paio);
  
  //tree.emplace(2,10);

  // Testing <<
  std::cout<<"Tree:\n"<<tree <<std::endl;
  
  // Testing copy semantic
  //bst<int,int> albero = tree;
  //bst<int,int> albero{tree};
  //std::cout<<"Albero : \n" <<albero <<std::endl;
  
  // Testing move semantic
  //bst<int,int> albero = std::move(tree);
  //bst<int,int> albero{std::move(tree)};
  //std::cout<<"Albero : \n" <<albero <<std::endl;
  //std::cout<<"Tree : \n" <<tree <<std::endl;
  
  // Testing find
  //if ( (*tree.find(2)).first == 2)
  //std::cout<<"print 2 if you found it: " <<(*tree.find(2)).first <<std::endl; 
  
  // Testing balance
  //tree.balance();
  //std::cout<<"Tree: \n"<<tree <<std::endl;
  
  // Testing clear
  //tree.clear();
  //std::cout <<tree <<std::endl;
  
  // Testing erase
  //tree.erase(45);//not present
  //tree.erase(14);
  //tree.erase(10);
  //tree.erase(6);
  //std::cout<<"Tree after erase:\n" <<tree <<std::endl;
  
  // Testing []
  //std::cout<<tree[2] <<std::endl;
  //tree[8] = 99;
  //std::cout<<tree <<std::endl;
  //tree[9] = 99;
  //std::cout<< tree <<std::endl;

}; // END OF MAIN




/* Incomplete swap_nodes function for the erase function, for the case of node to be swapped having 2 children.

  //when I call swap, I'm sure that "it" has 2 children
  void swap_nodes(iterator& it, iterator& next) {
    if ( next.get_curr()->parent == it.get_curr() ) {
      next.get_curr()->parent = it.get_curr()->parent;
      it.get_curr()->parent = next.get_curr();
      if ( am_I_left(it)) {
        if (no_child(next)) {
          next.get_curr()->parent->left.reset( it.get_curr()->right.release() );
          it.get_curr()->right.reset(nullptr);
          next.get_curr()->right.reset( it.get_curr() );
        }
        else if ( right_child(next) ) {
          next.get_curr()->parent->left.reset( it.get_curr()->right.release() );
          it.get_curr()->right.reset( next.get_curr()->right.release() );
          next.get_curr()->right.reset( it.get_curr() );
        }
      }
      else if ( am_I_right(it) ) {
        if (no_child(next)) {
          next.get_curr()->parent->right.reset( it.get_curr()->right.release() );
          it.get_curr()->right.reset(nullptr);
          next.get_curr()->right.reset( it.get_curr() );
        }
        else if ( right_child(next) ) {
          next.get_curr()->parent->left.reset( it.get_curr()->right.release() );
          it.get_curr()->right.reset( next.get_curr()->right.release() );
          next.get_curr()->right.reset( it.get_curr() );
        }
      }   
    }
 
    else if (am_I_root(it)) {
      root.reset( next.get_curr() );
      it.get_curr()->parent = next.get_curr()->parent;
      next.get_curr()->parent = nullptr; 
      

      if (am_I_left(next)) {   
        next.get_curr()->parent->left.reset( it.get_curr() ); 
        next.get_curr()->left.reset( it.get_curr()->left.release() );
        next.get_curr()->left->parent = next.get_curr();
        it.get_curr()->left.reset(nullptr);        
        if ( no_child(next) ) {
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(nullptr);
        }
        else if ( right_child(next) ) {
          auto next_rc = next.get_curr()->right.release();
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(next_rc);
          it.get_curr()->right->parent = it.get_curr();
        }     
      }
      
      else if (am_I_right(next)) {
        next.get_curr()->parent->right.reset( it.get_curr() ); 
        next.get_curr()->left.reset( it.get_curr()->left.release() );
        next.get_curr()->left->parent = next.get_curr();
        it.get_curr()->left.reset(nullptr); 
        if ( no_child(next) ) {
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(nullptr);
        }
        else if ( right_child(next) ) {
          auto next_rc = next.get_curr()->right.release();
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(next_rc);
          it.get_curr()->right->parent = it.get_curr();
        } 
      }
    }
    
    
    else if (am_I_left(it)) {
      auto i_p = it.get_curr()->parent;
      it.get_curr()->parent = next.get_curr()->parent;
      next.get_curr()->parent = i_p;
      if (am_I_left(next)) {
        it.get_curr()->parent->left.reset( next.get_curr()->parent->left.release() );
        next.get_curr()->parent->left.reset( next.get_curr() );
        next.get_curr()->left.reset( it.get_curr()->left.release() );
        next.get_curr()->left->parent = next.get_curr();
        it.get_curr()->left.reset(nullptr);

        if ( no_child(next) ) {
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(nullptr);
        }
        else if ( right_child(next) ) {
          auto next_rc = next.get_curr()->right.release();
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(next_rc);
          it.get_curr()->right->parent = it.get_curr();
        } 
      }
      else if (am_I_right(next)) {
        it.get_curr()->parent->right.reset( next.get_curr()->parent->left.release() );
        next.get_curr()->parent->left.reset( next.get_curr() );
        next.get_curr()->left.reset( it.get_curr()->left.release() );
        next.get_curr()->left->parent = next.get_curr();
        it.get_curr()->left.reset(nullptr);
        if ( no_child(next) ) {
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(nullptr);
        }
        else if ( right_child(next) ) {
          auto next_rc = next.get_curr()->right.release();
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(next_rc);
          it.get_curr()->right->parent = it.get_curr();
        } 
      }
    }
    
    
    else if (am_I_right(it)) {        //frist 3 lines for changing parents: at the end, "it" points to PN, and next points to PI
    auto i_p = it.get_curr()->parent;
    it.get_curr()->parent = next.get_curr()->parent;
    next.get_curr()->parent = i_p;
      if (am_I_left(next)) {
        it.get_curr()->parent->left.reset( next.get_curr()->parent->right.release() );
        // left when NEXT left; right when IT right
        next.get_curr()->parent->right.reset( next.get_curr() );     //right when IT right, left when IT left
        next.get_curr()->left.reset( it.get_curr()->left.release() );  
        next.get_curr()->left->parent = next.get_curr();
        it.get_curr()->left.reset(nullptr);
        if ( no_child(next) ) {
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(nullptr); 
        }
        else if ( right_child(next) ) {
          auto next_rc = next.get_curr()->right.release();
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(next_rc);
          it.get_curr()->right->parent = it.get_curr();
        } 
      }
      else if (am_I_right(next)) {
        it.get_curr()->parent->right.reset( next.get_curr()->parent->right.release() );
        next.get_curr()->parent->right.reset( next.get_curr() );
        next.get_curr()->left.reset( it.get_curr()->left.release() );
        next.get_curr()->left->parent = next.get_curr();
        it.get_curr()->left.reset(nullptr);
        if ( no_child(next) ) {
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(nullptr);
        }
        else if ( right_child(next) ) {
          auto next_rc = next.get_curr()->right.release();
          next.get_curr()->right.reset( it.get_curr()->right.release() );
          next.get_curr()->right->parent = next.get_curr();
          it.get_curr()->right.reset(next_rc);
          it.get_curr()->right->parent = it.get_curr();
        } 
      }
    }
  }

*/











