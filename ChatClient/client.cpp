#include "client.h"

const int g_ciBuffLength = 512;

ChatClient g_clientObj;

UINT __cdecl RecvServerThread(LPVOID pParam)
{
    g_clientObj.RecvServerMessage();
    return 0;
}

ChatClient::ChatClient():
    m_uiPort(0),
    m_socketClient(0)
{
}

ChatClient::ChatClient(unsigned int uiPort):
    m_uiPort(uiPort),
    m_socketClient(0)
{

}

ChatClient::~ChatClient()
{
    // close the socket
    int iResult = closesocket(m_socketClient);
    if (iResult == SOCKET_ERROR)
        cout << "close failed with error: " << WSAGetLastError();

    WSACleanup();
}

int ChatClient::Setup(unsigned int uiPort)
{
    m_uiPort = uiPort;

    int iResult;
    WSADATA wsaData;

    struct sockaddr_in clientService; 

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR)
    {
        cout << "WSAStartup failed with error: " << iResult << endl;
        return 1;
    }

    // Create a SOCKET for connecting to server
    m_socketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socketClient == INVALID_SOCKET)
    {
        cout << "socket failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    unsigned int addr = inet_addr("129.135.52.205");//"127.0.0.1"
    struct hostent *hp =gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);

    if( hp == NULL )
    {
        cout << "getting host adress failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
    clientService.sin_port = htons( m_uiPort );

    // Connect to server.
    iResult = connect( m_socketClient, (SOCKADDR*) &clientService, sizeof(clientService) );
    if (iResult == SOCKET_ERROR)
    {
        cout << "connect failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

   return 0;
}

int ChatClient::SendClientMessage(char * pcMessage)
{
    // Send an Message
    int iResult = send( m_socketClient, pcMessage, (int)strlen(pcMessage), 0 );
    if (iResult == SOCKET_ERROR)
    {
        cout << "send failed with error: " << WSAGetLastError() << endl;
        closesocket(m_socketClient);
        WSACleanup();
        return 1;
    }
    
  

    return 0;
}

int ChatClient::ShutDown()
{
    // shutdown the connection since no more data will be sent
    int iResult = shutdown(m_socketClient, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        cout << "shutdown failed with error: " << WSAGetLastError() << endl;
        closesocket(m_socketClient);
        WSACleanup();
        return iResult;
    }
    return iResult;
}


int ChatClient::RecvServerMessage()
{
    int iResult = 0;
    char recvbuf[g_ciBuffLength] = "";

    // Receive until the peer closes the connection
    do {
        iResult = recv(m_socketClient, recvbuf, g_ciBuffLength, 0);
        if ( iResult > 0 )
        {
            cout << "-->" << recvbuf << endl;
            //break; //break is the culprit
        }
        else if ( iResult == 0 )
            cout << "Connection closed" << endl;
        else
            cout << "recv failed with error: " << WSAGetLastError() << endl;

    } while( iResult > 0 );

    return 0;
}

int main(int argc, char* argv[])
{
    unsigned int uiPort = 8084;

    // parse the arguments
    for( int i = 1; i < argc; i++)
    {
        char *pcValue = NULL;
        if( ( argv[i][0] == 'p' ) || ( argv[i][0] == 'P' ) )
        {
            pcValue = &argv[i][2];
            uiPort = atoi(pcValue);
        }
    }

    cout << "This is a client Connecting to port " << uiPort << endl;
    cout << "Type exit to quit" << endl;
    cout << "--------------------------------------------" << endl;

    int iResult = 0;

    // setup the client
    iResult = g_clientObj.Setup(uiPort);
    if(iResult!= 0)
    {
        cout << "Client setup is failed" << endl;
        return iResult;
    }

    char sBuffer[g_ciBuffLength] = {0};

    AfxBeginThread(RecvServerThread, 0);

    while( gets_s( sBuffer, g_ciBuffLength-1 ) )
    {
        if( strcmp(sBuffer, "exit"))
            g_clientObj.SendClientMessage(sBuffer);
        else
        {
            g_clientObj.ShutDown();
            break;
        }
    }

    return iResult;
}