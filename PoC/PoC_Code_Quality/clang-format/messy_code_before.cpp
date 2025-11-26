#include <iostream>
#include <vector>

int main() {
    std::vector<int> v={1,2,3,4,5};
    for(auto& i:v){
        if(i%2==0){
            std::cout<<"Even: "<<i<<std::endl;
        }else{
            std::cout<<"Odd: "<<i<<std::endl;
        }
    }
    return 0;
}