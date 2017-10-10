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
#include <unordered_map>

using namespace std;

class Server {

  private:
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    char buff[256];

  public:
    Server(int siteID, unordered_map<int, pair<string, string> >& address);
    static void * HandleClient(void *args);
    static void* startTweet(void *args);
    static void* acceptRequest(void *args);
    static void doSomethingWithReceivedData(string& message);
    static void block(string& input);
    static void unblock(string& input);
    static void view(string& input);
    void start();
  private:
    int id;
    unordered_map<int, pair<string, string> > addr;
};

#endif
