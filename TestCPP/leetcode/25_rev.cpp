#include<iostream>
#include<tuple>

using namespace std;


struct ListNode{
  int val;
  ListNode* next;
  ListNode(int val):val(val),next(nullptr){}

};

tuple<ListNode*,ListNode*> cst(){

  ListNode* head = new ListNode(2);
  head->next = new ListNode(1);
  head->next->next = new ListNode(4);
  ListNode* end = head->next->next;
  return make_tuple(head,end);

}

tuple<ListNode*,ListNode*> reverse(ListNode* start,ListNode* end){

        if (start == nullptr)exit(-1);
        if (start == end){
            return make_tuple(start,end);
        }
        ListNode* suf = start;
        ListNode* index = start->next;
        ListNode* pre = index->next;
        while(index != end && pre != nullptr){
            if(suf == start)
                suf->next = nullptr;
            index->next = suf;
            suf = index;
            index = pre;
            pre = pre->next; 
        }
        index->next = suf;
        end = index;
        suf = nullptr;
        pre = nullptr;
        return make_tuple(end,start);

}

int main(int argc,char* argv[]){

  tuple<ListNode*, ListNode*> tmp = cst();
  ListNode* head = std::get<0>(tmp);
  ListNode* end = std::get<1>(tmp);
  for(auto index = head;index!=nullptr;index=index->next){
    cout<<index->val<<";";
  }
  cout<<endl;
  //tmp = reverse(head,end);
  tmp = reverse(head,nullptr);
  head = std::get<0>(tmp);
  end = std::get<1>(tmp);
  //cout<<"Test: "<<end->val<<endl;
  for(auto index = head;index!=nullptr;index=index->next){
    cout<<index->val<<";";
  }

}


