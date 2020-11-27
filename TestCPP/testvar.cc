#include <iostream>

int c = 0;

class wget {
private:
    int exm{0};
    const double price{0};
    //const double price;
    //int exm = 0;
    const static int var{2};
    int& ref{c};

public:
    
    wget(int& a){
        std::cout<<"Constructor"<<std::endl;
    }
    wget(const wget& rhs):price(rhs.price),ref(rhs.ref){
        exm = rhs.exm;
        std::cout<<"Copy Constructor"<<std::endl;
    }
    wget(wget&& rhs):price(rhs.price),ref(rhs.ref){
        exm = rhs.exm;
        std::cout<<"Move Constructor"<<std::endl;
    }
    ~wget(){
        std::cout<<"Destructor"<<std::endl;
    }

    friend std::ostream& operator<<(std::ostream& out,const wget&);
};

//const int wget::var = 2;

class proxy{
public:
    //proxy(const proxy&){}
    //proxy(proxy&&){}
    proxy& operator=(const proxy& ){return *this;}
    ~proxy(){}
};

std::ostream& operator<<(std::ostream& out,const wget& wg){
    out<<"Exm: "<<wg.exm<<"\n";
    out<<"Const Var: "<<wg.price<<"\n";
    out<<"Static Var: "<<wget::var;
    return out;
}

int main(int argc,char* argv[]){
    int b = 1;
    wget a = wget(b);
    //wget b = wget();
    std::cout<<a<<std::endl;
    //std::cout<<b<<std::endl;
    proxy d;
    return 0;
}