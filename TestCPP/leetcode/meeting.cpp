#include<iostream>
#include<vector>
#include<algorithm>
using namespace std;

void getSumB(vector<int> a,int target){
    sort(a.begin(),a.end());//排序 O(NlgN)
    size_t left = 0;
    size_t right = a.size()-1;
    while(left < right ){
        if(a[left]+a[right] == target){
            cout<<a[left]<<"+"<<a[right]<<"="<<target<<endl;
            ++left;
            --right;
        }
        else if(a[left]+a[right] < target){
            ++left;
        }else{
            --right;
        }
    }
}

void func(vector<int> a,vector<int> b){
    if(a.empty() || b.empty())return;
    //O(M*NlgN)
    for(const auto& target:b){   //O(M)
        getSumB(a,target);
    }

}

int main(int argc, char *argv[]){

  vector<int> a={1,5,3,2,7,1,6,4,3,2};
  vector<int> b={7,5};
  func(a,b);

}
