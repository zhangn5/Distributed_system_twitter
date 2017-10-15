//
//  LogAndDictionary.cpp
//  
//
//  Created by Ni Zhang on 10/2/17.
//
//

#include "LogAndDictionary.hpp"
#include <set>
#include <iostream>
#include <string>
using namespace std;


const string Event::covToString() const{
    string s = "";
    s += to_string(userID);
    s += " ";
    s += to_string(clock);
    s += " ";
    int len = op.length();//record the length of op for parsing
    s += to_string(len);
    s += " ";
    s += op;
    return s;
}


void Log::updateLog(string op, int clock, int userID){
    Event newEvent;
    newEvent.op = op;
    newEvent.clock = clock;
    newEvent.userID = userID;
    Events.insert(newEvent);
}


void Dictionary::Insert(int user1, int user2){
    Entry.insert(make_pair(user1, user2));
}
void Dictionary::Delete(int user1, int user2){
    if(Entry.find(make_pair(user1, user2)) != Entry.end()){
        Entry.erase(make_pair(user1,user2));
    }else{
        cerr << "Not able to unblock because it's not blocked";
    }
}
void Dictionary::updatePartialLog(string op, int clock, int userID){
    Event newEvent;
    newEvent.op = op;
    newEvent.clock = clock;
    newEvent.userID = userID;
    PL.insert(newEvent);
}



