#include "server.h"

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

const int g_ciBufferLength=512;
bool g_bExitThread = false;

ChatServer g_serverObj;

int timestamp(char *pcTimeStamp)
{
    char cTime[128] = {0};
    char cDate[128] = {0};

    _tzset();
    _strtime_s(cTime, 128);
    _strdate_s(cDate, 128);

    strcat_s(cTime, " ");
    strcat_s(cTime, cDate);
    strcpy_s(pcTimeStamp, g_ciBufferLength, cTime);

    return 0;
}

UINT __cdecl RecvClientThread( LPVOID pParam )
{
    SOCKET socketClient= (SOCKET)pParam;

    g_serverObj.RecieveClientMessage(socketClient);

    return 0;
}

UINT __cdecl ClientAcceptThread( LPVOID pParam)
{
    while(1)
    {
        g_serverObj.AcceptClients();
        if(g_bExitThread == true)
            break;
    }

    return 0;
}

ChatServer::ChatServer():
    m_ServerSocket(0),
    m_uiServerPort(0)
{
    m_LogFile.open("ServerClient.log");
}

ChatServer::ChatServer(unsigned int uiServerPort):
    m_ServerSocket(0),
    m_uiServerPort(uiServerPort)
{
    m_LogFile.open("ServerClient.log");
}

ChatServer::~ChatServer()
{
    // close the socket
    int iResult = closesocket(m_ServerSocket);
    if (iResult == SOCKET_ERROR)
        cout << "close socket failed with error: " << WSAGetLastError() << endl;

    m_LogFile.close();

    // Do the clean up
    WSACleanup();
}

int ChatServer::Setup(unsigned int uiPort)
{
    m_uiServerPort = uiPort;

    // Initialize Winsock
    WSADATA wsaData;
    int iResult = 0;

    sockaddr_in service;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) 
    {
        cout << "WSAStartup() failed with error: " << iResult << endl;
        return 1;
    }
    
    // Create a SOCKET for listening for incoming connection requests.
    m_ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_ServerSocket == INVALID_SOCKET) 
    {
        cout << "socket function failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(m_uiServerPort);

    iResult = bind(m_ServerSocket, (SOCKADDR *) & service, sizeof (service));
    if (iResult == SOCKET_ERROR) 
    {
        cout << "bind function failed with error " << WSAGetLastError() << endl;
        iResult = closesocket(m_ServerSocket);
        if (iResult == SOCKET_ERROR)
            cout << "closesocket function failed with error " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    
    // Listen for incoming connection requests 
    // on the created socket
    if (listen(m_ServerSocket, SOMAXCONN) == SOCKET_ERROR)
        cout << "listen function failed with error: " << WSAGetLastError() << endl;

    cout << "Listening on socket..." << endl;

    return 0;
}


int ChatServer::AcceptClients()
{
    SOCKET socketClient = SOCKET_ERROR ;

    while( socketClient == SOCKET_ERROR )
    {
        // Accept a new client connection.
        socketClient = accept(m_ServerSocket, NULL, NULL);
        if (socketClient == INVALID_SOCKET)
        {
            cout << "socket function failed with error: " << WSAGetLastError() << endl;
            break;
        }
        else
            m_vecClientSockets.push_back(socketClient);
    }

    if( socketClient != INVALID_SOCKET )
    {
        char cTimeStamp[g_ciBufferLength] = {0};
        timestamp(cTimeStamp);

        m_LogFile << "Client : " << socketClient << " connected at " << cTimeStamp << endl;
        cout << "Client : " << socketClient << " connected at " << cTimeStamp << endl;

        AfxBeginThread(RecvClientThread, (LPVOID) socketClient);
    }

    return socketClient;
}

int ChatServer::RecieveClientMessage(SOCKET socketClient)
{
    int iResult = 0;

    char cMessage[g_ciBufferLength] = {0};
    char cTimeStamp[g_ciBufferLength] = {0};
    char cHello[g_ciBufferLength] = "Hello!\t";

    do
    {
        char recvBuf[g_ciBufferLength] = {0};
        iResult = recv(socketClient, recvBuf, g_ciBufferLength, 0);
        if( iResult > 0 )
        {
            cMessage[0] = '\0';
            cTimeStamp[0] = '\0';
            
            timestamp(cTimeStamp);

            //strcat_s(cMessage, cHello);
            strcat_s(cMessage, recvBuf);
            strcat_s(cMessage, " ");
            strcat_s(cMessage, cTimeStamp);
            cout << recvBuf << endl;
            m_LogFile << "Client : " << socketClient << " recived message at " << cTimeStamp << endl;
            SendServerMessage(socketClient, cMessage);
        }
        else if( iResult == 0 )
        {
            cTimeStamp[0] = '\0';
            timestamp(cTimeStamp);

            m_LogFile << "Client : " << socketClient << " connection closed at " << cTimeStamp << endl;
            cout << "Client : " << socketClient << " connection closed at " << cTimeStamp << endl;
            break;
        }
        else
            cout << "recv failed with error: " << WSAGetLastError() << endl;

    } while (iResult > 0);


    return iResult;
}

int ChatServer::SendServerMessage(SOCKET sClients, char *sMessage)
{
    int iRetCode = SOCKET_ERROR;
    char cTimeStamp[g_ciBufferLength] = { 0 };

    for (int i = 0; i < m_vecClientSockets.size(); i++)
    {        
        if (m_vecClientSockets[i] != sClients)
        {
            int iLen = strlen(sMessage);
            iRetCode = send(m_vecClientSockets[i], sMessage, iLen, 0);

            if (iRetCode == SOCKET_ERROR)
            {
                cout << "send failed with error: " << WSAGetLastError() << endl;
                closesocket(m_vecClientSockets[i]);
                timestamp(cTimeStamp);
                m_LogFile << "Client : " << m_vecClientSockets[i] << " connection closed at " << cTimeStamp << endl;
                cout << "Client : " << m_vecClientSockets[i] << " connection closed at " << cTimeStamp << endl;
                //m_vecClientSockets.erase(i);
                WSACleanup();
                return iRetCode;
            }

        }
    }
    

    return iRetCode;
}


int ChatServer::ShutDown()
{
    // shutdown the connection since no more data will be sent
    int iResult = shutdown(m_ServerSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR)
    {
        cout << "shutdown failed with error: " << WSAGetLastError() << endl;
        closesocket(m_ServerSocket);
        WSACleanup();
        return iResult;
    }

    closesocket(m_ServerSocket);
    WSACleanup();
    return iResult;
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

    
    cout << "Connect to the server pc port " << uiPort << endl;
    cout << "Type exit to quit" << endl;
    cout << "--------------------------------------------" << endl;


    int iResult = 0;

    iResult= g_serverObj.Setup(uiPort);
    if(iResult!= 0)
    {
        cout << "Server setup is failed" << endl;
        return iResult;
    }

    AfxBeginThread(ClientAcceptThread, 0);

    char sBuffer[g_ciBufferLength] = {0};

    while(1)
    {
        gets_s( sBuffer, g_ciBufferLength-1 );

        if(!strcmp(sBuffer, "exit") )
        {
            g_bExitThread = true;
            //g_serverObj.ShutDown();

            break;
        }
    }

    return iResult;
}

