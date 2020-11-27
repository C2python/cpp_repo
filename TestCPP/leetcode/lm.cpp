#include<iostream>
#include<string>
#include<vector>
#include<algorithm>

using namespace std;


class Solution {
public:
    
    class trieNode{
        public:
        char value;
        vector<trieNode*> links;
        trieNode(const char ch){
            value = ch;
            for (auto i=0;i<=('z'-'a');++i){
                links[i] = nullptr;
            }
        }
        ~trieNode(){
            for (auto i=0;i<=('z'-'a');++i){
                    if (links[i] != nullptr)delete links[i];
                }
        }
        int insert(const string& res,size_t position = 0){
            if (res[position] == '\0') return 0;
            if (links[res[position]-'a'] == nullptr){
                try{
                    links[res[position]-'a'] = new trieNode(res[position]);
                }catch(const bad_alloc& e){
                    //links[res[position]-'a'] = nullptr;
                    return -1;
                }
            }
            int r = links[res[position]-'a']->insert(res,position+1);
            return r;
        }
        char get(){
            return value;
        }
        trieNode(const trieNode& rhs)=default;
        trieNode& operator=(const trieNode& rhs)=default;
        trieNode(trieNode&& rhs){
            value = rhs.value;
            for (auto i=0;i<rhs.links.size();++i){
                links[i] = rhs.links[i];
                rhs.links[i] = nullptr;
            }
        }
        trieNode& operator=(trieNode&& rhs){
            value = rhs.value;
            for (auto i=0;i<rhs.links.size();++i){
                links[i] = rhs.links[i];
                rhs.links[i] = nullptr;
            }
            return *this;
        }
    };
    
    class trie{
        public:
        trieNode* root;
        trie(){
            root = new trieNode('#');
        }
        int insert(const string& str){
            if (str.empty())return 0;
            int r = root->insert(str);
            return r;
        }
        string getCommonPrefix(){
            string res="";
            auto index = root;
            while(std::count_if(index->links.begin(),index->links.end(),[](const trieNode* ptr) {return ptr != nullptr;}) != 1){
                auto tmp = std::find_if(index->links.begin(),index->links.end(),[](const trieNode* ptr){return ptr!=nullptr;});
                index = *tmp;
                res.append(1,index->get());
            }
            return res;
        }
        ~trie(){
            delete root;
        }
    };
    
    string longestCommonPrefix(vector<string>& strs) {
        trie tre;
        for (auto i=0;i<strs.size();++i){
            tre.insert(strs[i]);
        }
        return tre.getCommonPrefix();
    }
};


int main(int argc,char *argv[]){

Solution sl;

vector<string> str;

str.push_back("flower");
str.push_back("flow");
str.push_back("flight");

string res = sl.longestCommonPrefix(str);
cout<<res<<endl;

}
