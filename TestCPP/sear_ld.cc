#include<iostream>
#include<vector>
using namespace std;

class Solution {
public:
    
    int lower_bound(vector<vector<int>>& matrix,int target){
        
        int start = 0;
        int end = matrix.size()-1;
        
        int mid = (start + end)/2;
        while (start <= end){
            if (target == matrix[mid][0]){
                break;
            }
            if (target < matrix[mid][0]){
                end = mid - 1;
            }else{
                start = mid + 1;
            }
            mid = (start + end)/2;
        }
        if (start > matrix.size()-1 || end < 0)return -1;
        mid = end;
        return mid;
        
    }
    
    bool find_mid(vector<vector<int>>& matrix, int row, int target){
        
        int start = 0;
        int end = matrix[row].size()-1;
        int mid = (start+end)/2;
        while (start <= end){
            if (matrix[row][mid] == target){
                return true;
            }
            if (target < matrix[row][mid]){
                end = mid - 1;
            }else{
                start = mid + 1;
            }
            mid = (start + end)/2;
        }
        return false;
        
    }
    
    bool searchMatrix(vector<vector<int>>& matrix, int target) {
        if (matrix.empty() || matrix[0].empty())return false;
        if (target < matrix[0][0] || target > matrix[matrix.size()-1][matrix[0].size()-1])return false;
        
        int upper = 0;
        upper = lower_bound(matrix,target);
        std::cout<<"UPPER: "<<upper<<std::endl;
        if (upper<0){
            return false;
        }
        
        return find_mid(matrix,upper,target);
        
    }
};

int main(int argc,char *argv[]){

  vector<vector<int>> matrix{{1,2,5,7},{10,11,16,20},{23,30,34,50}};
  Solution sl;
  bool res = sl.searchMatrix(matrix,3);
  std::cout<<"Res: "<<res<<std::endl;

}
