//
//  LogAndDictionary.hpp
//  
//
//  Created by Ni Zhang on 10/2/17.
//
//

#ifndef LogAndDictionary_hpp
#define LogAndDictionary_hpp

#include <stdio.h>
#include <tweet.hpp>
#include <unordered_set>
#include <string>
using namespace std;

class Event{
public:
    int userID;
    int clock;
    string op;

    string covToString();

};

struct comp {
    bool operator()(Event& a, Event& b) {
        return a.covToString() < b.covToString();
    }
};

class Log {
public:
    unordered_set<Event, comp> Events;
    
    void updateLog(string op, int clock, int userID);

};

class Dictionary{
public:
    //pair of user and its follower to state the status of block
    //the second is blocked from viewing the first user
    unordered_set<pair<int, int> > Entry;
    unordered_set<Event, comp> PL;
    
    void Insert(int user1, int user2);
    void Delete(int user1, int user2);
    void updatePartialLog(string op, int clock, int userID);
};



#endif /* LogAndDictionary_hpp */
