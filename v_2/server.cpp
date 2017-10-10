#include "server.h"
#include <unordered_set>

using namespace std;

struct cAddrSSock {
    int serverSock;
    struct sockaddr_in clientAddr;
    int id;
    unordered_map<int, pair<string, string> > addr;
    cAddrSSock(int a, struct sockaddr_in b, int c, unordered_map<int, pair<string, string> > d) {
        serverSock = a;
        clientAddr = b;
        id = c;
        addr = d;
    }
};

//Actually allocate clients
Server::Server(int siteID, unordered_map<int, pair<string, string> >& address) {
    MyThread::InitMutex();
    int yes = 1;
    addr = address;
    id = siteID;
    string ip = address[id].first;
    string port = address[id].second;
    cerr << "my ip: "<<ip <<", my port: "<<stoi(port)<<"\n";
    //Init serverSock and start listen()'ing
    serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    bzero((char*) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(stoi(port));
    
    //Avoid bind error if the socket was not close()'d last time;
    setsockopt(serverSock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
    
    if(bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        cerr << "Failed to bind";
    listen(serverSock, 5);
}

void Server::start() {
    MyThread *t1 = new MyThread();
    MyThread *t2 = new MyThread();
    struct cAddrSSock* cass =  new cAddrSSock(serverSock, clientAddr, id, addr);
    t2->Create((void *) startTweet, cass);
    t1->Create((void *) acceptRequest, cass);
    t2->Join();
    t1->Join();
}

void * Server::acceptRequest(void *args) {
    struct cAddrSSock* cass = (struct cAddrSSock*) args;
    int serverSock = cass->serverSock;
    struct sockaddr_in clientAddr = cass->clientAddr;
    socklen_t cliSize = sizeof(clientAddr);
    while(1) {
        MyThread *t = new MyThread();
        int fd = accept(serverSock, (struct sockaddr *) &clientAddr, &cliSize);
        if (fd > 0) t->Create((void *) HandleClient, &fd);
        
    }
}

void *Server::startTweet(void *args) {
    struct cAddrSSock* cass = (struct cAddrSSock*) args;
    int serverSock = cass->serverSock;
    struct sockaddr_in clientAddr = cass->clientAddr;
    int id = cass->id;
    unordered_map<int, pair<string, string> > addr = cass->addr;
    unordered_set<int> fdv;
    while(1) {
        cout << ">> ";
        int n;
        string cmd, input;
        cin >> cmd;
        if(cmd == "tweet") {
            getline(cin, input);
            input = input.substr(1);
            input = to_string(id)+" "+"tweet "+input;
            const char* message = input.c_str();
            fdv.clear();//has to clear fd set, because some site can be down and up anytime
            for(auto & i : addr) {
                if (i.first == id) continue;
                struct hostent *server;
                const char * ip = i.second.first.c_str();
                server = gethostbyname(ip);
                struct sockaddr_in svrAdd;
                int listenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                bzero((char *) &svrAdd, sizeof(svrAdd));
                svrAdd.sin_family = AF_INET;
                bcopy((char *) server->h_addr, (char *) &svrAdd.sin_addr.s_addr, server->h_length);
                svrAdd.sin_port = htons(stoi(i.second.second));
                if (connect(listenFd, (struct sockaddr *) &svrAdd, sizeof(svrAdd))>=0) {
                    fdv.insert(listenFd);
                }
            }
            for(auto & i : fdv) {
                n = send(i, message, strlen(message), 0);
                //cout << n << " bytes sent." << endl;
            }
        } else if (cmd == "block") {
            getline(cin, input);
            input = input.substr(1);
            block(input);
        }
        else if (cmd == "unblock") {
            getline(cin, input);
            input = input.substr(1);
            unblock(input);
        }
        else if (cmd == "view") view(input);
        else cerr << "invalid command\n";
    }
}

void *Server::HandleClient(void *args) {
    int * fd = (int *)args;
    char buffer[256-25];
    int index;
    int n;
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        n = recv(*fd, buffer, sizeof(buffer), 0);
        if (n < 0) break;//cout << "error on recv\n";
        else if (n==0) break;
        else if (n > 0) {
            //MyThread::LockMutex("stdout");
            //lock_guard<std::mutex> lock(mtx);
                cerr << "Received message: "<< buffer << "\n>> ";
            //MyThread::UnlockMutex("stdout");
            string tmp = string(buffer);
            doSomethingWithReceivedData(tmp);
        }
    }
    //End thread
    return NULL;
}

void Server::doSomethingWithReceivedData(string& message) {
    
}

void Server::block(string& input) {
    
}

void Server::unblock(string& input) {
    
}

void Server::view(string& input) {
    
}

