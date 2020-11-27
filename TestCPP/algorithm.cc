#include<iostream>

using namespace std;


int maxproduct(int length){


    if(length == 0){
        return 0;
    }else if(length == 1){
        return 1;
    }else if(length == 2){
        return 2;
    }else if(length == 3){
        return 3;
    }

    int* product = new int[length+1];
    product[0] = 0;
    product[1] = 1;
    product[2] = 2;
    product[3] = 3;
    int max = 0;
    int tmp = 0;
    for(int i=4;i<=length;i++){
        max = 0;
        for(int j=1;j<=i/2;++j){
            tmp = product[j] * product[i-j];
            if(max < tmp)
                max = tmp;
        }
        product[i] = max;
    }
    max = product[length];
    delete []product;
    return max;

}


int main(int argc,char* argv[]){

    for(int i=4;i<20;++i){
        std::cout<<maxproduct(i)<<std::endl;
    }

}