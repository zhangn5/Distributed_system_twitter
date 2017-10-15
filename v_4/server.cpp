#include "server.h"
#include "tweet.hpp"
#include "LogAndDictionary.hpp"

#include <set>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include <istream>
#include <string>
#include <time.h>
#include <vector>
#include <locale>
#include <iomanip>

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
Server::Server(int siteID, unordered_map<int, pair<string, string> >& address, int numSites_) {
    MyThread::InitMutex();
    int yes = 1;
    addr = address;
    userID = siteID;
    numSites = numSites_;
    
    //Each site(server) store the log containing tweet, block and unblock events
    //Init the log, dictionary, timetable and clock for Wuu Bernstein algorithm
    Log log;
    Dictionary dict;
    timeTable = vector<vector<int> >(numSites, vector<int>(numSites, 0));
    clock = 0;
    
    string ip = address[userID].first;
    string port = address[userID].second;
    cerr << "my ip: "<<ip <<", my port: "<<stoi(port)<<"\n";
    //Init serverSock and start listen()'ing
    serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    bzero((char*) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(stoi(port));
    
    //Avoid bind error if the socket was not close()'d last time;
    setsockopt(serverSock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
    
    if(::bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        cerr << "Failed to bind";
    listen(serverSock, 5);
}

void Server::start() {
    MyThread *t1 = new MyThread();
    MyThread *t2 = new MyThread();
    struct cAddrSSock* cass =  new cAddrSSock(serverSock, clientAddr, userID, addr);
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
            getline(cin, input);//get the following tweet words
            input = input.substr(1);
            
            int tweetlen = input.length();//the length of tweet for reading
            
            clock++;
            timeTable[userID-1][userID-1] = clock;
            
            //add the local time and UTC time in the tweet op
            time_t localtime;
            struct tm *utctm;
            time(&localtime);//local time
            utctm = gmtime(&localtime);//UTC time
            
            string op = "tweet " + to_string(tweetlen) + "#" + input + "#" + string(ctime(&localtime)) + "#" + string(asctime(utctm)) + "#";
            
            log.updateLog(op, clock, userID);
            string tmsg = convertTo1DTT(timeTable);
            
            //construct sending log content NP
            vector<string> NP;
            for(auto it = log.Events.begin(); it != log.Events.end(); it++){
                for(int j = 0; j < numSites; j++){//if unblocked could be sent
                    if(!hasRec(timeTable, *it, j)){
                        NP.push_back((*it).covToString());
                    }
                }
            }
            
            //construct sending partial log content PLNP
            
            vector<string> PLNP;
            for(auto it = dict.PL.begin(); it != dict.PL.end(); it++){
                for(int j = 0; j < numSites; j++){ //if unblocked could be sent
                    if(!hasRec(timeTable, *it, j)){
                        PLNP.push_back((*it).covToString());
                    }
                }
            }
            
            string npmsg = convertTo1DNP(NP);
            string plnpmsg = convertTo1DNP(PLNP);
            
            int tmsglen = tmsg.length();
            int npmsglen = npmsg.length();
            int plnpmsglen = plnpmsg.length();
            
            string ownview = string(ctime(&localtime)) + " " + to_string(id)+" "+"tweet "+input;
            cout << ownview << endl;
            
            //construct sending message with timetable ,np and partial log np
            string sending = to_string(userID) +" "+ to_string(tmsglen) + " " + tmsg + to_string(npmsglen) +" " + npmsg + to_string(plnpmsglen) + " " + plnpmsg;
            const char* sendout = sending.c_str();
            
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
                n = send(i, sendout, strlen(sendout), 0);
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
        else if (cmd == "view") view();
        else if (cmd == "viewlog") viewlog();

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
        else if (n==0) break;//connection lost
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
    //parse the message into timetable, np and partial log np
    string tmsg, npmsg, plnpmsg;
    stringstream ss(message);
    string cont;
    int start = 0;
    ss >> cont;
    int userIDk = stoi(cont);
    start += cont.length() + 1;
    ss >> cont;
    start += cont.length() + 1;
    tmsg = message.substr(start, stoi(cont));
    start += stoi(cont);
    stringstream left1(message.substr(start));
    left1 >> cont;
    start += cont.length() + 1;
    npmsg = message.substr(start, stoi(cont));
    start += stoi(cont);
    stringstream left2(message.substr(start));
    left2 >> cont;
    plnpmsg = message.substr(start + cont.length() + 1, stoi(cont));

    //userID can be read from the tweet
    vector<Event> npv = recvNP(npmsg);
    vector<Event> plnpv = recvNP(plnpmsg);
    vector<Event> NE;

    vector<vector<int> > tmsgTable = recvTT(tmsg);
    
    //NE
    for(int i = 0; i < plnpv.size(); i++){
        if(!hasRec(timeTable, plnpv[i], userID)){
            NE.push_back(plnpv[i]);
        }
    }
    //update entries
    
    for(int i = 0; i < NE.size(); i++){
        stringstream ss(NE[i].op);
        string cont;
        ss >> cont;
        if(cont == "block"){
            ss >> cont;
            dict.Entry.insert(make_pair(NE[i].userID, stoi(cont)));
        }
    }
    for(auto it = dict.Entry.begin(); it != dict.Entry.end(); it++){
        for(int j = 0; j < NE.size(); j++){
            if(NE[j].op == "unblock "+ to_string((*it).second) && NE[j].userID == (*it).first){
                dict.Entry.erase(it);
            }
        }
    }
    
    
    for(int i = 0; i < npv.size(); i++){
        log.Events.insert(npv[i]);
    }
    for(int i = 0; i < numSites; i++){
        timeTable[userID-1][i] = max(timeTable[userID-1][i], tmsgTable[userIDk-1][i]);
    }
    for(int i = 0; i < numSites; i++){
        for(int j = 0; j < numSites; j++){
            timeTable[i][j] = max(timeTable[i][j], tmsgTable[i][j]);
        }
    }
    //update partial log
    for(int i = 0; i < NE.size(); i++){
        dict.PL.insert(NE[i]);
    }
    for(auto it = dict.PL.begin(); it != dict.PL.end(); it++){
        bool good = false;
        for(int j = 0; j < numSites; j++){
            if(!hasRec(timeTable, *it, j)){
                good = true;
                break;
            }
        }
        if(good == false){
            dict.PL.erase(it);
        }
    }

}

void Server::block(string& input) {
    clock++;
    timeTable[userID-1][userID-1] = clock;
    int blockID = stoi(input);
    string op = "block " + input;
    log.updateLog(op, clock, userID);
    dict.updatePartialLog(op, clock, userID);
    dict.Insert(userID, blockID);
    
}

void Server::unblock(string& input) {
    clock++;
    timeTable[userID-1][userID-1] = clock;

    string op = "unblock " + input;
    log.updateLog(op, clock, userID);
    dict.updatePartialLog(op, clock, userID);

    int unblockID = stoi(input);
    dict.Delete(userID, unblockID);
}

//view displays the timeline, i.e. the entire set of tweets, sorted in descending order of the time field(most recent tweets appear first), excluding tweets that the user is blocked from seeing.

void Server::view() {
    //look for the blocked user from userID in hte dictionary
    unordered_set<int> blockeduser;
    for(auto it = dict.Entry.begin(); it != dict.Entry.end(); it++){
        if((*it).second == userID){
            blockeduser.insert((*it).first);
        }
    }
    //collect tweets from the local log
    vector<Tweet> vt;
    for(auto it = log.Events.begin(); it != log.Events.end(); it++){
        stringstream ss((*it).op);
        string content;
        ss >> content;
        if(content == "tweet"){
            int tweetlen = 0;
            Tweet tweet;
            tweet.userID = (*it).userID;
            getline(ss, content, '#');
            tweetlen = stoi(content);
            tweet.message = (*it).op.substr(7 + (to_string(tweetlen)).length(), tweetlen);
            stringstream left((*it).op.substr(7 + (to_string(tweetlen)).length() + tweetlen));
            getline(left, content, '#');
            
            struct tm tm;
            istringstream tt1(content);
            tt1.imbue(locale("de_DE.utf-8"));
            tt1 >> get_time(&tm, "%a %b %d %H:%M:%S %Y");
            tweet.local = mktime(&tm);

            getline(left, content, '#');
            istringstream tt2(content);
            tt2.imbue(locale("de_DE.utf-8"));
            tt2 >> get_time(&tm, "%a %b %d %H:%M:%S %Y");
            tweet.utc = mktime(&tm);
            vt.push_back(tweet);
        }
    }
    sort(vt.begin(), vt.end(), comparetime);
    cout << "Viewable tweets:" << endl;
    for (auto it=vt.begin(); it!=vt.end(); ++it){
        if(blockeduser.find((*it).userID) == blockeduser.end()){
            cout << ctime(&(*it).local) << " " << to_string((*it).userID) << " " << (*it).message << endl;
        }
    }
    
}
void Server::viewlog(){
    for(auto it = log.Events.begin(); it != log.Events.end(); it++){
        cout << (*it).covToString() << endl;
    }
}


bool Server::hasRec(vector<vector<int> > timeTable, Event eR, int user){
    return timeTable[user][eR.userID-1] >= eR.clock;
}

string Server::convertTo1DTT(vector<vector<int> >& arr){
    string str = "";
    for (int i = 0; i < numSites; i++) {
        for(int j = 0; j < numSites; j++){
            str += (to_string(arr[i][j]) + " ");
        }
    }
    return str;
}
string Server::convertTo1DNP(vector<string>& NP){
    string str = "";
    for (int i = 0; i < NP.size() - 1; i++) {
            str += (NP[i] + " ");
    }
    str += NP[NP.size() - 1];
    return str;
}

vector<Event> Server::recvNP(string& npmsg){
    vector<Event> ve;
    Event e;
    int posi = 0; //position of first space
    int posj = 0;
    int type = 0;
    while((posj = npmsg.find(' ', posi)) != -1){
        if(type == 0){
            e.userID = stoi(npmsg.substr(posi, posj-posi));
        }else if (type == 1){
            e.clock = stoi(npmsg.substr(posi, posj-posi));
        }else if(type == 2){
            int len = stoi(npmsg.substr(posi, posj-posi));
            e.op = npmsg.substr(posj, len);
            posj += len + 1;
            ve.push_back(e);
        }
        type = (type + 1) % 3;
        posi = posj + 1;
    }
    return ve;
}

vector<vector<int> > Server::recvTT(string& tmsg){
    vector<vector<int> > arr;
    stringstream ss(tmsg);
    int value;
    int count = 0;
    int i = 0;
    while(ss >> value){
        while(count < numSites){
            arr[i].push_back(value);
            count++;
        }
        i++;
        count = 0;
    }
    return arr;
}






