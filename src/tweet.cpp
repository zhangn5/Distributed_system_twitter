//
//  tweet.cpp
//  
//
//  Created by Ni Zhang on 10/2/17.
//
//

#include "tweet.hpp"
#include <time.h>

using namespace std;

Tweet::Tweet(){
    
}

Tweet::Tweet(int id, string msg){
    this->id = id;
    this->string = msg;
    time_t timec;
    time(&timec);
    this->time = timec;
}
