#ifndef _server_h_
#define _server_h_

#include <iostream>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "mythread.h"
#include "LogAndDictionary.hpp"
#include "tweet.hpp"
#include <unordered_map>
#include <set>
#include <vector>
#include <string>

using namespace std;

class Server {

  private:
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    char buff[256];

  public:
    Server(int siteID, unordered_map<int, pair<string, string> >& address, int numSites);
    static void * HandleClient(void *args);
    void* startTweet(void *args);
    static void* acceptRequest(void *args);
    void doSomethingWithReceivedData(string& message);
    void block(string& input);
    void unblock(string& input);
    void view();
    void viewlog();
    bool hasRec(vector<vector<int> > timeTable, Event eR, int user);
    string convertTo1DTT(vector<vector<int> >& arr);
    string convertTo1DNP(vector<string>& NP);
    vector<Event> recvNP(string& npmsg);
    vector<vector<int> > recvTT(string& tmsg);
    void start();
  
  private:
    int userID;
    Log log;
    Dictionary dict;
    int numSites;
    int clock;
    vector<vector<int> > timeTable;
    unordered_map<int, pair<string, string> > addr;
    
};

#endif
