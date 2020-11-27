#include<iostream>
#include<vector>

using namespace std;

int main(int argc,char* argv[]){
    vector<int> vec1;
    vector<int> tmp;
    vec1.push_back(1);
    vec1.push_back(2);
    vec1.swap(tmp);
    vec1.push_back(3);
    for (const auto& it:vec1){
        std::cout<<it<<std::endl;
    }
    
    return 0;
}