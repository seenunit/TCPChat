#include <cstdio>
#include <conio.h>
#include <iostream>

#include <afxwin.h>
#include <WinSock2.h>

using namespace std;

class ChatClient
{
public: 
    ChatClient();
    ChatClient(unsigned int uiPort);
    ~ChatClient();

    // Setup client 
    int Setup(unsigned int uiPort);
    // Send message to server
    int SendClientMessage(char * pcMessage);
    // Shutdown client
    int ShutDown();
    // Reciver Serve message and display
    int RecvServerMessage();
    
private:
    unsigned int m_uiPort;
    SOCKET m_socketClient;
};