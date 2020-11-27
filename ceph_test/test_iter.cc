#include <iostream>
#include <algorithm>
#include <list>
#include <gtest/gtest.h>
#include <string>

/*
** 定义list，Node积累，主要包含前后指针
** 定义两个操作函数：分别是在此节点后插入，其次是将本身从list链表中移除
*/
struct list_base{
public:
    list_base* next;
    list_base* prev;
    list_base(){
        next = this;
        prev = this;
    }
    virtual void insert(list_base* e){
        e->next = this;
        e->prev = prev;
        prev->next = e;
        prev = e;
    }
    virtual void unlink(){
        next->prev = prev;
        next->prev->next = next;
        next = this;
        prev = this;
    }
    virtual ~list_base(){}
};

/*
** 存放数据
*/
template<typename T>
class Node: public list_base{
public:
    T ele;
public:
    Node(T ele):ele(ele){}
};

/*
** 定义迭代器iterator
*/
template<typename T>
class buffers_iterator{
public:

    list_base* cur;
    using _self = buffers_iterator<T>;
    using _Node = Node<T>;
public:
    
    using value_type = T;
    using reference = std::add_lvalue_reference_t<T>;
    using pointer = std::add_pointer_t<T>;
    using difference_type = std::ptrdiff_t;

    /* 
    * 由于是链表结构，因此只支持std::forward_iterator_tag 
    * 连续容器使用：std::random_access_iterator_tag
    */
    using iterator_category = std::forward_iterator_tag;

    buffers_iterator(list_base* const p)
    : cur(p){
    }
    buffers_iterator()=default;
    template<typename U>
    buffers_iterator(const buffers_iterator<U>& other)
    : cur(other.cur){
    }
    reference operator*(){
        return static_cast<_Node*>(cur)->ele;
    }
    pointer operator->(){
        return &(static_cast<_Node*>(cur)->ele);
    }
    _self& operator++(){
        cur = cur->next;
        return *this;
    }
    _self operator++(int){
        const auto temp(*this);
        ++*this;
        return temp;
    }
    template<typename U>
    _self& operator=(const buffers_iterator<U>& other){
        cur = other.cur;
        return *this;
    }
    bool operator==(const buffers_iterator& other){
        return cur==other.cur;
    }
    bool operator!=(const buffers_iterator& other){
        return !(*this==other);
    }
};

/*
** 定义const iterator
** iterator与const iterator区别：const reference及const pointer，以禁止通过此迭代器对存储的数据进行更改
*/
template<typename T>
class buffers_const_iterator{
public:
    list_base* cur;
    using _self = buffers_const_iterator<T>;
    using _Node = const Node<T>;
public:
    using value_type = T;
    using reference = std::add_lvalue_reference_t<std::add_const_t<T>>;
    using pointer = std::add_pointer_t<std::add_const_t<T>>;
    using difference_type = std::ptrdiff_t;
    /* 
    * 由于是链表结构，因此只支持std::forward_iterator_tag 
    * 连续容器使用：std::random_access_iterator_tag
    */
    using iterator_category = std::forward_iterator_tag;

    using iterator = buffers_iterator<T>;

    buffers_const_iterator(list_base* const p)
    : cur(p){
    }

    buffers_const_iterator(const iterator& p)
    :cur(p.cur){}

    buffers_const_iterator()=default;
    template<typename U>
    buffers_const_iterator(const buffers_const_iterator<U>& other)
    : cur(other.cur){
    }
    reference  operator*(){
        return static_cast<_Node*>(cur)->ele;
    }
    pointer operator->(){
        return &(static_cast<_Node*>(cur)->ele);
    }
    _self& operator++(){
        cur = cur->next;
        return *this;
    }
    _self operator++(int){
        const auto temp(*this);
        ++*this;
        return temp;
    }
    template<typename U>
    _self& operator=(const buffers_const_iterator<U>& other){
        cur = other.cur;
        return *this;
    }
    bool operator==(const buffers_const_iterator& other){
        return cur==other.cur;
    }
    bool operator!=(const buffers_const_iterator& other){
        return !(*this==other);
    }
};

/*
** 定义iterator与const iterator之间的比较函数
*/
template<typename _Val>
inline bool
  operator==(const buffers_iterator<_Val>& __x,
         const buffers_const_iterator<_Val>& __y)
  { return __x.cur == __y.cur; }

template<typename _Val>
  inline bool
  operator!=(const buffers_iterator<_Val>& __x,
         const buffers_const_iterator<_Val>& __y)
  { return __x.cur == __y.cur; }

/*
** 定义list容器，在容器内声明iterator及const iterator两种迭代器类型
** 定义接口函数
*/
template<typename T>
class buffers{
    using _Node = Node<T>;
    list_base _root;
public: 
    using iterator = buffers_iterator<T>;
    using const_iterator = buffers_const_iterator<T>;
    using value_type = T;

    buffers(){
        _root.next = &_root;
        _root.prev = &_root;
    }
    buffers(const buffers&) = delete;
    buffers(buffers&& other){
          _root.next = other._root.next == &other._root ? &_root : other._root.next;
          _root.prev = other._root.prev == &other._root ? &_root : other._root.prev;
	      other._root.next = &other._root;
          other._root.prev = &other._root;
    }
    void push_back(value_type val) {
        list_base* item = new _Node(val);
        _root.insert(item);
      }

    void push_front(value_type val) {
        list_base* item = new _Node(val);
        begin().cur->insert(item);
    }
    const_iterator begin() const {
	      return const_iterator(_root.next);
    }
    const_iterator end() const {
	    return const_iterator(&_root);
    }
    iterator begin() {
	    return iterator(_root.next);
    }
    iterator end() {
	    return iterator(&_root);
    }

    iterator back() {
        return iterator(_root.prev);
    }

    const_iterator back() const{
        return iterator(_root.prev);
    }

    void pop_back(){
        if ( _root.prev == &_root){
            return;
        }
        _erase(_root.prev);
    }

    void pop_front(){
        if (_root.next == &_root){
            return;
        }
        _erase(_root.next);
    }

    iterator insert(iterator pos,const value_type& _val){
        list_base* item = new _Node(_val);
        pos.cur->insert(item);
        return pos.cur->prev;
    }

    iterator erase(iterator pos){
        iterator tmp = pos++;
        _erase(tmp);
        return pos;
    }

    void _erase(iterator pos){
        pos.cur->unlink();
        delete pos.cur;
    }

    bool empty(){
        return _root.next == &_root;
    }

    ~buffers(){
        list_base* head = _root.next;
        list_base* t = _root.next;
        while (head!=&_root){
            head = t->next;
            delete t;
            t = head;
        }
    }
};


TEST(INT,PUSH){
    buffers<int> t1;
    t1.push_back(1);
    EXPECT_EQ(*(t1.back()),1);
    t1.push_front(2);
    EXPECT_EQ(*(t1.begin()),2);
}
TEST(INT,POP){
    buffers<int> t1;
    t1.push_back(2);
    t1.pop_back();
    EXPECT_TRUE(t1.empty());
}
TEST(INT,ITERATOR){
    buffers<int> t1;
    t1.push_front(1);
    t1.insert(t1.begin(),2);
    EXPECT_EQ(*(t1.begin()),2);
    t1.erase(t1.begin());
    EXPECT_EQ(*(t1.begin()),1);
}

/*
** Test String
*/

TEST(STRING,PUSH){
    buffers<std::string> t1;
    t1.push_back("abc");
    EXPECT_EQ(*(t1.back()),"abc");
    t1.push_front("dc");
    EXPECT_EQ(*(t1.begin()),"dc");
}
TEST(STRING,POP){
    buffers<std::string> t1;
    t1.push_back("dc");
    t1.pop_back();
    EXPECT_TRUE(t1.empty());
}
TEST(STRING,ITERATOR){
    buffers<std::string> t1;
    t1.push_front("as");
    t1.insert(t1.begin(),"dc");
    EXPECT_EQ(*(t1.begin()),"dc");
    t1.erase(t1.begin());
    EXPECT_EQ(*(t1.begin()),"as");
}

TEST(ALGORITHM,FIND_IF){
    buffers<int> t1;
    t1.push_back(12);
    t1.push_back(10);
    t1.push_back(5);
    t1.push_back(20);
    auto it = std::find_if(t1.begin(),t1.end(),[](auto e){return e%5==0;});
    EXPECT_EQ(*it,10);
}

int main(int argc,char* argv[]){

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    
}