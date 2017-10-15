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


bool comparetime(const Tweet &i,const Tweet &j) {  return difftime(i.utc,j.utc) > 0.0;}
