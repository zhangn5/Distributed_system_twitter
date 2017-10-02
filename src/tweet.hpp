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
    int id;
    string message;
    time_t time;
};
#endif /* tweet_hpp */
