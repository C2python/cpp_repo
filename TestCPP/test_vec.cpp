#include<iostream>
#include<vector>
#include<algorithm>
#include<string>
using namespace std;

struct node{
int key;
node(int key,int val):key(key),j(val){}
~node(){}
node(const node& rhs){
key = rhs.key;
j = rhs.j;
}
node& operator=(const node& rhs){
key = rhs.key;
j = rhs.j;
}

node(node&& rhs)=default;
node& operator=(node&& rhs)=default;
/*
bool operator<(const node& rhs){
  if(key<rhs.key)return true;
  return false;
}
*/

friend ostream& operator<<(ostream& os,const node& rhs);
private:
int j;
};

ostream& operator<<(ostream& os,const node& rhs){
os<<rhs.key;
return os;
}

bool comp(const node& a, const node& b){
   if(a.key<b.key) return true;
   return false;
}


 bool isMatch(){
        string s = "aaa";
        string p = "aaaa";
        int len1 = s.size();
        int len2 = p.size();
        bool dp[len1+1][len2+1];
        int tmp = dp[1][1] == true? 1:0;
        std::cout<<1<<","<<1<<":"<<tmp<<std::endl;
        for (int i=0;i<len1+1;++i){
            dp[i][0] = false;
        }
        dp[0][0] = true;
        for (int j=1;j<len2;++j){
            if (p[j]=='*'){
               dp[0][j+1] = dp[0][j-1];
            }            
        }
        for (int i=0;i<len1;++i){
            for (int j=0;j<len2;++j){
                if (p[j]!='*'){
                    if (s[i] == p[j] || p[j] == '.'){
                        
                        dp[i+1][j+1] = dp[i][j];
                        int tmp = dp[i+1][j+1]== true? 1:0;
                        std::cout<<i+1<<","<<j+1<<":"<<tmp<<std::endl;
                    }else{
                        dp[i+1][j+1] = false;
                    }
                }else{
                    dp[i+1][j+1] = j>0 && (dp[i+1][j-1] || (dp[i][j-1] && (s[i] == p[j-1] || p[j-1] == '.')) || (dp[i][j+1] && (s[i] == p[j-1] || p[j-1] == '.')) );
                }
            }
        }
        return dp[len1][len2];
    }

int main(int argc,char* argv[]){

//int a = -23;
//std::cout<<a%10<<std::endl;

/*
vector<node> temp;
temp.push_back(node(10,4));
temp.push_back(node(1,2));
temp.push_back(node(5,11));
temp.push_back(node(3,5));
temp.push_back(node(19,6));
sort(temp.begin(),temp.end(),comp);
for(const auto& i:temp){
  cout<<i<<endl;
}
*/
auto res=isMatch();
}
