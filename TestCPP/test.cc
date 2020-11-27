#include<iostream>
using namespace std;


class A{
int i;
int j;
const int cnt = 10;
public:
A(int val):j(i),i(val){
//cnt = 10;
}
};

int main(int argc,char* argv[]){

A* ptr = new A(10);

}
