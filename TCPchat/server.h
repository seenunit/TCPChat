#include <cstdio>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <list>
#include <time.h>

#include <afxwin.h>
#include <WinSock2.h>
#include <vector>


using namespace std;

class ChatServer
{
public:
    ChatServer();
    ChatServer(unsigned int uiServerPort);
    ~ChatServer();

    // setup the server
    int Setup(unsigned int uiPort);
    // Accepts the client messages
    int AcceptClients();
    // recieve clients message
    int RecieveClientMessage(SOCKET sClients);
    // sends server message
    int SendServerMessage(SOCKET sClients, char *sMessage);
    //ShutDown the connection
    int ShutDown();

private:
    SOCKET m_ServerSocket;
    unsigned int m_uiServerPort;
    ofstream m_LogFile;
    vector <SOCKET> m_vecClientSockets;
};

