#include "server.h"

using namespace std;

//Actually allocate clients
vector<Client> Server::clients;
int yes = 1;
Server::Server(int siteID, unordered_map<int, pair<string, string> >& address) {
    id = siteID;
    string ip = address[id].first;
    string port = address[id].second;
    cout << "my ip: "<<ip <<", my port: "<<port<<"\n";
    MyThread::InitMutex();
    //Init serverSock and start listen()'ing
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverAddr, 0, sizeof(sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(stoi(port));
    
    //Avoid bind error if the socket was not close()'d last time;
    setsockopt(serverSock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
    
    if(bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(sockaddr_in)) < 0)
        cerr << "Failed to bind";
    listen(serverSock, 5);
    //addClient(address);
    //notifyAll(address);
}

void Server::addClient(unordered_map<int, pair<string, string> >& address) {
    for(auto & i : address) {
        if (i.first==id) continue;
        Client *c = new Client();
        c->SetId(i.first);
        c->sock=stoi(i.second.second);
        Server::clients.push_back(*c);
        struct sockaddr_in c_addr;
        memset(&c_addr, 0, sizeof(sockaddr_in));
        c_addr.sin_family = AF_INET;
        c_addr.sin_addr.s_addr = inet_addr("localhost");
        c_addr.sin_port = htons(stoi(i.second.second));
        while(!connect(c->sock, (struct sockaddr *) &c_addr, sizeof(sockaddr_in)));
    }
}

/*
void Server::addClient(int numSites) {
    for(int i = 0; i < numSites; i++) {
        if (i == my_id) continue;
        Client *c;
        MyThread *t;
        socklen_t cliSize = sizeof(sockaddr_in);
        c->sock = accept(serverSock, (struct sockaddr *) &clientAddr, &cliSize);
        
    }
}
*/
/*
void Server::notifyAll(unordered_map<int, pair<string, string> >& address) {
    
}
*/
void Server::AcceptAndDispatch() {
    
    MyThread *sendThread = new MyThread();
    sendThread->Create((void *) Server::echoRequest, NULL);
    
    Client *c;
    MyThread *t;
    socklen_t cliSize = sizeof(sockaddr_in);
    while(1) {
        c = new Client();
        t = new MyThread();
        
        //Blocks here;
        c->sock = accept(serverSock, (struct sockaddr *) &clientAddr, &cliSize);
        
        if(c->sock < 0) {
            cerr << "Error on accept";
        }
        else {
            t->Create((void *) Server::HandleClient, c);
        }
    }
}

void *Server::echoRequest(void *args) {
    while(1) {
        int n;
        string input;
        cout << "Enter text: ";
        cin >> input;
        input = "req "+input;
        const char* message = input.c_str();
        //Acquire the lock
        MyThread::LockMutex("'echoRequest()'");
        for(size_t i=0; i<clients.size(); i++) {
            cout <<i+1 << ": "<<Server::clients[i].sock<<"\n";
            n = send(Server::clients[i].sock, message, strlen(message), 0);
            cout << n << " bytes sent." << endl;
        }
        //Release the lock
        MyThread::UnlockMutex("'echoRequest()'");
    }
}

//Static
void *Server::HandleClient(void *args) {
    
    //Pointer to accept()'ed Client
    Client *c = (Client *) args;
    char buffer[256-25], message[256];
    int index;
    int n;
    /*
    //Add client in Static clients <vector> (Critical section!)
    MyThread::LockMutex((const char *) c->name);
    
    //Before adding the new client, calculate its id. (Now we have the lock)
    c->SetId(Server::clients.size());
    sprintf(buffer, "Client n.%d", c->id);
    c->SetName(buffer);
    cout << "Adding client with id: " << c->id << endl;
    Server::clients.push_back(*c);
    
    MyThread::UnlockMutex((const char *) c->name);
    */
    while(1) {
        memset(buffer, 0, sizeof buffer);
        n = recv(c->sock, buffer, sizeof buffer, 0);
        
        //Client disconnected?
        /*
        if(n == 0) {
            cout << "Client " << c->name << " diconnected" << endl;
            close(c->sock);
            
            //Remove client in Static clients <vector> (Critical section!)
            MyThread::LockMutex((const char *) c->name);
            
            index = Server::FindClientIndex(c);
            cout << "Erasing user in position " << index << " whose name id is: "
            << Server::clients[index].id << endl;
            Server::clients.erase(Server::clients.begin() + index);
            
            MyThread::UnlockMutex((const char *) c->name);
            
            break;
        }
        else if(n < 0) {
            cerr << "Error while receiving message from client: " << c->name << endl;
        }
         */
        //else {
        string tmp = string(message);
        if (tmp.substr(0,3)=="req") {
            snprintf(message, sizeof message, "<%s>: %s", c->id, buffer);
            cout << "Receive echo request from Server "<< c->id << ": " << message << endl;
            Server::echoResponse(message, c);
        } else {
            //message = message.substr(4);
            cout << "Server "<<c->id<<" reply: "<<message<<"\n";
        }
        //}
    }
    
    //End thread
    return NULL;
}

void *Server::echoResponse(char* message, Client* c) {
    int i = FindClientIndex(c);
    int n = send(Server::clients[i].sock, message, strlen(message), 0);
    cout << "Echo response: " << n << " bytes sent." << endl;
}

void Server::SendToAll(char *message) {
    int n;
    
    //Acquire the lock
    MyThread::LockMutex("'SendToAll()'");
    
    for(size_t i=0; i<clients.size(); i++) {
        n = send(Server::clients[i].sock, message, strlen(message), 0);
        cout << n << " bytes sent." << endl;
    }
    
    //Release the lock
    MyThread::UnlockMutex("'SendToAll()'");
}

void Server::ListClients() {
    for(size_t i=0; i<clients.size(); i++) {
        cout << clients.at(i).name << endl;
    }
}

/*
 Should be called when vector<Client> clients is locked!
 */
int Server::FindClientIndex(Client *c) {
    for(size_t i=0; i<clients.size(); i++) {
        if((Server::clients[i].id) == c->id) return (int) i;
    }
    cerr << "Client id not found." << endl;
    return -1;
}

