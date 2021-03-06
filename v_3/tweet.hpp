//
//  tweet.hpp
//  
//
//  Created by Ni Zhang on 10/2/17.
//
//

#ifndef tweet_hpp
#define tweet_hpp

#include <stdio.h>
#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <time.h>

using namespace std;

class Tweet {
public:
    int userID;
    string message;
    time_t local;
    time_t utc;
    
    //record time in two format with the first being UTC to compare and second being local time for printing
};

bool comparetime(const &Tweet i,const &Tweet j);


#endif /* tweet_hpp */
