#include "server.h"

using namespace std;

//Actually allocate clients
int yes = 1;
Server::Server(int siteID, unordered_map<int, pair<string, string> >& address) {
    addr = address;
    id = siteID;
    string ip = address[id].first;
    string port = address[id].second;
    cerr << "my ip: "<<ip <<", my port: "<<stoi(port)<<"\n";
    MyThread::InitMutex();
    //Init serverSock and start listen()'ing
    serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    //fcntl(serverSock, F_SETFL, O_NONBLOCK);
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


void* Server::addClient() {
    vector<pair<string, string> > v(addr.size()+1);
    for(auto & i : addr) v[i.first]=i.second;
    int added = 0;
    for(int i = 1; i < v.size(); i++) {
        //cout << "Adding client: "<<i<<"\n";
        if (i==id) continue;
        //socklen_t len = sizeof(clntAdd);
        
        int listenFd, portNo=stoi(v[i].second);
        const char* serverIP = v[i].first.c_str();
        struct hostent *server;
        server = gethostbyname(serverIP);
        struct sockaddr_in svrAdd;
        
        listenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        bzero((char *) &svrAdd, sizeof(svrAdd));
        svrAdd.sin_family = AF_INET;
        bcopy((char *) server->h_addr, (char *) &svrAdd.sin_addr.s_addr, server->h_length);
        svrAdd.sin_port = htons(portNo);
        int fd = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));
        
        if(fd < 0) {
            i--;
            continue;
        } else {
            cout << "connect "<<portNo<<"\n";
            Client *c = new Client();
            c->SetId(i);
            c->sock=portNo;
            clients.push_back(*c);
            //string str = "Connect from " + to_string(id);
            //const char* message = str.c_str();
            //send(c->sock, message, strlen(message), 0);
            //socketfd.push_back(fd);
            //cout << socketfd.size()<<" ";
            added++;
        }
        cout << "client "<<i<<" added\n";
        if(added==addr.size()-1) {
            break;
        }
    }
    cout<<"All clients are added: ";
    for(auto & i : clients) cout << i.sock <<" ";
    cout << "\n";
    
    //MyThread *sendThread = new MyThread();
    //sendThread->Create((void *)echoRequest(), NULL);
    if (id==1||id==2) {
        acceptRequest();
        //pthread_t tid1;
        //pthread_create(&tid1, NULL, (void *(*)(void *))acceptRequest(), NULL);
    } else {
        echoRequest();
        //pthread_t tid2;
        //pthread_create(&tid2, NULL, (void *(*)(void *))echoRequest(), NULL);
    }
    //pthread_join(tid1, NULL);
    //pthread_join(tid2, NULL);
    //}
}

void * Server::acceptRequest() {
    Client *c;
    MyThread *t;
    socklen_t cliSize = sizeof(clientAddr);
    cout <<"Start to accept\n";
    while(1) {
        c = new Client();
        t = new MyThread();
        //struct clientAddr;
        cout << "Accepting...\n";
        //Blocks here;
        //fcntl(serverSock, F_SETFL, O_NONBLOCK);
        c->sock = accept(serverSock, (struct sockaddr *) &clientAddr, &cliSize);
        //int connFd = accept(serverSock, (struct sockaddr *) &clientAddr, &cliSize);
        //c->id = FindClientIndex(c);
        if(c->sock < 0) {
            //cerr << "Error on accept";
        } else {
            cout << "accepted: " << clientAddr.sin_port <<" "<<ntohs(clientAddr.sin_port)<<"\n";
            t->Create((void *)HandleClient(c), c);
        }
    }
}

void *Server::echoRequest() {
    cout << "Start Echo Request Service\n";
    while(1) {
        int n;
        string input;
        cout << "Enter text: ";
        cin >> input;
        cout << "Entered text is: "<<input<<"\n";
        input = to_string(id)+" "+"req "+input;
        const char* message = input.c_str();
        //Acquire the lock
        MyThread::LockMutex("'echoRequest()'");
        for(size_t i=0; i < clients.size(); i++) {
            cout << i+1 << ": "<<clients[i].sock<<"\n";
            struct hostent *server;
            server = gethostbyname("127.0.0.1");
            struct sockaddr_in svrAdd;
            int listenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            bzero((char *) &svrAdd, sizeof(svrAdd));
            svrAdd.sin_family = AF_INET;
            bcopy((char *) server->h_addr, (char *) &svrAdd.sin_addr.s_addr, server->h_length);
            svrAdd.sin_port = htons(clients[i].sock);
            int fd = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));
            
            n = send(fd, message, strlen(message), 0);
            cout << n << " bytes sent." << endl;
        }
        //Release the lock
        MyThread::UnlockMutex("'echoRequest()'");
    }
}

//Static

void *Server::HandleClient(void *args) {
    cout <<"Handling client\n";
    //Pointer to accept()'ed Client
    Client *c = (Client *) args;
    
    char buffer[256-25], message[256];
    int index;
    int n;
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        n = recv(c->sock, buffer, sizeof(buffer), 0);
        
        string tmp = string(buffer);
        if(tmp.size()==0) break;
        if (!isdigit(tmp[0])) {
            //close(c->sock);
            break;
        }
        int clientId = stoi(tmp);
        string response = "Echo "+tmp;
        strncpy(message, response.c_str(), sizeof(message));
        message[sizeof(message) - 1] = 0;
        
        Client client = clients[FindClientIndex(clientId, c)];
        //cout << id << " recvs " << tmp <<" from " <<client.id<< "\n";
        if (tmp.size()<=3) continue;
        if (!(tmp.substr(0,3)=="req" || tmp.substr(0,3)=="res")) continue;
        if (tmp.substr(0,3)=="req") {
            //snprintf(message, sizeof(message), "<%s>: %s", client.id, buffer);
            cout << id <<" receives echo request from Server "<< client.id << ": " << tmp << endl;
            //echo response
            //n = send(c->sock, message, strlen(message), 0);
            n = send(htons(7001), message, strlen(message), 0);
            //n = send(htons(7002), message, strlen(message), 0);
            cout << "Echo response: " << n << " bytes sent." << endl;
        }
    }
    //End thread
    return NULL;
}

void *Server::echoResponse(char* message, Client* c) {
    //int i = FindClientIndex(c);
    int n = send(c->sock, message, strlen(message), 0);
    cout << "Echo response: " << n << " bytes sent." << endl;
    //return NULL;
}

void Server::SendToAll(char *message) {
    int n;
    
    //Acquire the lock
    MyThread::LockMutex("'SendToAll()'");
    
    for(size_t i=0; i<clients.size(); i++) {
        n = send(clients[i].sock, message, strlen(message), 0);
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
int Server::FindClientIndex(int clientId, Client *c) {
    for(size_t i=0; i<clients.size(); i++) {
        if(clients[i].id == clientId) {
            clients[i].sock = c->sock;
            return (int) i;
        }
    }
    cerr << "Client id not found." << endl;
    return -1;
}

