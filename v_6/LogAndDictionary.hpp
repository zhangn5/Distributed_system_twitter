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
#include "tweet.hpp"
#include <set>
#include <string>
#include <fstream>

using namespace std;

class Event{
public:
    int userID;
    int clock;
    string op;

    const string covToString() const;
    const string convToStringForViewlog() const;
    const string convToStringForStoring() const;

};

struct comp {
    bool operator()(const Event& a, const Event& b) {
        return a.covToString() < b.covToString();
    }
};

class Log {
public:
    //Log();
    set<Event, comp> Events;
    void writeToDisk();
    void readFromDisk();
    void updateLog(string op, int clock, int userID);
    void updateLogUnblock(string op, int clock, int userID, int unblockID);


};

class Dictionary{
public:
    //pair of user and its follower to state the status of block
    //the second is blocked from viewing the first user
    //Dictionary();
    set<pair<int, int> > Entry;
    set<Event, comp> PL;
    void writeToDisk();
    void readFromDisk();
    void Insert(int user1, int user2);
    set<pair<int, int> >::iterator Erase(set<pair<int, int> >::iterator& it);
    void Delete(int user1, int user2);
    void updatePartialLog(string op, int clock, int userID);
};



#endif /* LogAndDictionary_hpp */
