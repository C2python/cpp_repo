#include<utility>
#include<iostream>

int test(int a,int b){
    int c = a+b;
    std::cout<<"Run Test"<<c<<std::endl;
    return c;
}

/* 无法通过编译
template<typename Fn, typename... Args>
decltype(Fn(Args...)) test_decltype3(Fn f, Args... args) {
    std::cout<<"Run Test_Decltype3"<<std::endl;
    auto res = f(args...);
    return res;
}
*/

template<typename Fn, typename... Args>
auto test_decltype2(Fn f, Args... args) -> decltype(f(args...)){
    std::cout<<"Run Test_Decltype2"<<std::endl;
    auto res = f(args...);
    return res;
}

template<typename Fn,typename... Args>
decltype(std::declval<Fn>()(std::declval<Args>()...)) test_decltype1(Fn f,Args... args){
    std::cout<<"Run Test_Decltype1"<<std::endl;
    auto res = f(args...);
    return res;
}

int main(int agrc,char *argv[]){

    auto res0 = test_decltype1(test,1,2);
    auto res1 = test_decltype2(test,1,3);
    std::cout<<"Main Res0: "<<res0<<std::endl;
    std::cout<<"Main Res1: "<<res1<<std::endl;

    return 0;

}