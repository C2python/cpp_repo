#include<iostream>
#include<memory>
using namespace std;

template<typename T,size_t N>
int getLen(T (&a)[N]){
return N;
}

void test(int a[],int length){

//size_t pp = getLen(a);
int *p=new int[length];

delete[] p;
}

int main(){

int a[10] = {1,5,3,83,5,73,4,55,3,2};
size_t length = getLen(a);
test(a,length);
}
