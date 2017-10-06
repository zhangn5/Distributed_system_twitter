#include <iostream>
#include "mythread.h"
#include "server.h"
#include <fstream>
#include <unordered_map>

using namespace std;

int main(int argc, char* argv[]) {
    cout << "Running!" << endl;
    if (argc<2) {
        cerr << "Please enter site ID to start server.\n";
        exit(1);
    } else if (argc<3) {
        cerr << "Please enter number of sites.\n";
        exit(1);
    }
    
    int siteID = atoi(argv[1]);
    int numSites = atoi(argv[2]);
    cout << "site ID: " <<siteID <<", number of sites: "<<numSites<<"\n";
    if (siteID>numSites || siteID<=0) {
        cerr << "siteID should be in range: [1, "<<numSites<<"].\n";
        exit(1);
    }
    ifstream input("hosts.txt");
    string str;
    unordered_map<int, pair<string, string> > address;
    for(int i = 0; i < numSites; i++) {
        if(!getline(input, str)) {
            cerr << "Not enough hosts in hosts.txt\n";
            exit(1);
        }
        string ip;
        string port;
        int pos = str.find(" ");
        ip = str.substr(0, pos);
        while(str[pos]==' ') pos++;
        port = str.substr(pos);
        address[i+1]=make_pair(ip, port);
    }
    cout << "Setting up server...\n";
    Server *s;
    s = new Server(siteID, address);
    //s->addClient(numSites);
    MyThread *addClientThread = new MyThread();
    addClientThread->Create((void *) s->addClient(), NULL);
    addClientThread->Join();
    //s->addClient();
    //MyThread *acceptThread = new MyThread();
    //acceptThread->Create((void *) s->AcceptAndDispatch(), NULL);
    //acceptThread->Join();
    
    return 0;
}
