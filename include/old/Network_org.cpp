/* New operating system independend Socket class started 16.11.2012 Sebastian 
 * Tauscher  Neu schreiben eines Platform unabhängigen SDKs?*/


#if defined(_WIN32) && !defined(__CYGWIN__)
  #include <windows.h>
  #include <winsock2.h> 
  #include <iphlpapi.h>
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <sys/time.h>
#endif

#include <stdio.h>
#include "Network.h"
#define INVALID_SOCKET -1;
#define SOCKET_ERROR -1;
// INVALID_Socket == -1

CNetwork::CNetwork()
{
    mhSocket             = -1;
    mhUDPSocket          = -1;
    mhUDPBroadcastSocket = -1;
    mnLastError          = 0;
    maErrorStr[0]        = 0;
}


bool CNetwork::InitWinsock()
{
	
	//This should work on both windows and linux
	#if defined(_WIN32) && !defined(__CYGWIN__)
    WORD    wVersionRequested = MAKEWORD(2,2);
    WSADATA wsaData;

    // Initialize WinSock and check version

    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        SetErrorString();
        return false;
    }
    if (wsaData.wVersion != wVersionRequested)
    {	
        SetErrorString();
        return false;
    }
	
	#endif
    return true;
} // InitWinsock


bool CNetwork::Connect(char* pServerAddr, int nPort)
{
    mnLastError   = 0;
    maErrorStr[0] = 0;

	//Added INITWinSock just neccesarry when the OS is Windows
	#if defined(_WIN32) && !defined(__CYGWIN__)
    if (InitWinsock() == false)
    {
        return false;
    }
	#endif

    // Connect to QTM RT server.

    mhSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in sAddr;

    // First check if the address is a dotted number "A.B.C.D"

    sAddr.sin_addr.s_addr = inet_addr(pServerAddr);
    if (sAddr.sin_addr.s_addr == INADDR_NONE)
    {
        // If it wasn't a dotted number lookup the server name
		//printf("not dotted hostname...");
        hostent *psHost = gethostbyname(pServerAddr);
        if (!psHost)
        {
            //printf("Error looking up host name."); 
            close(mhSocket);
            return false;
        }
        sAddr.sin_addr = *((in_addr*)psHost->h_addr_list[0]);
    }
    
    sAddr.sin_port = htons(nPort);
    sAddr.sin_family = AF_INET;
   // printf("want to connect...%d on port %d\n", mhSocket, nPort);
	int check = connect(mhSocket,  reinterpret_cast<sockaddr*>(&sAddr), sizeof(sAddr));
	//printf("connected to %d\n", check);
    if (check == -1)
    {
        SetErrorString();
        close(mhSocket);
         //printf("failed...\n");
        return false;
    }

    // Disable Nagle's algorithm
    //Changed from char to int 

    int bNoDelay = 1;

    if (setsockopt(mhSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay)))
    {
        SetErrorString();
		 printf("failed to disable nagle algorith,m..\n");
        close(mhSocket);
        return false;
    }
    return true;
} // Connect


void CNetwork::Disconnect()
{
    // Try to shutdown gracefully

    shutdown(mhSocket, SHUT_WR);
    int nRecved = 1;
    char pData[500];
    while (nRecved > 0)
    {
        //
        // There shouldn't be anything left to receive now, but check just to make sure
        //
        nRecved = recv(mhSocket, pData, sizeof(pData), 0);
    }
    close(mhSocket);
    close(mhUDPSocket);
    close(mhUDPBroadcastSocket);
    mhSocket             = INVALID_SOCKET;
    mhUDPSocket          = INVALID_SOCKET;
    mhUDPBroadcastSocket = INVALID_SOCKET;
    #if defined(_WIN32) && !defined(__CYGWIN__)
    WSACleanup();
    #endif
} // Disconnect


bool CNetwork::Connected()
{
    return mhSocket != INVALID_SOCKET;
}

bool CNetwork::CreateUDPSocket(int nUDPPort, bool bBroadcast)
{
    if (nUDPPort > 1023)
    {
		
		//Not yet implemented...
		
        /*int tempSocket = INVALID_SOCKET;

        // Create UDP socket for data streaming
        sockaddr_in RecvAddr;
        RecvAddr.sin_family = AF_INET;
        RecvAddr.sin_port = htons(nUDPPort);
        RecvAddr.sin_addr.s_addr = INADDR_ANY;
	
        tempSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (tempSocket != -1)
        {
            u_long argp = 1;
            // Make socket unblocking. windows ioctlsocket(...,..,...)
            if (ioctl(tempSocket, FIONBIO , &argp) == 0)
            {
                if (bind(tempSocket, (sockaddr *) &RecvAddr, sizeof(RecvAddr)) != -1)
                {
                    if (bBroadcast)
                    {
                        char nBroadcast = 1;
                        if (setsockopt(tempSocket, SOL_SOCKET, SO_BROADCAST, &nBroadcast,
                                       sizeof(nBroadcast)) == 0)
                        {
                            mhUDPBroadcastSocket = tempSocket;
                            return true;
                        }
                        else
                        {
                            printf(maErrorStr, sizeof(maErrorStr), "Failed to set socket options for UDP server socket."); 
                        }
                    }
                    else
                    {
                        mhUDPSocket = tempSocket;
                        return true;
                    }
                }
                else
                {
                    printf(maErrorStr, sizeof(maErrorStr), "Failed to bind UDP server socket."); 
                }
            }
            else
            {
                printf(maErrorStr, sizeof(maErrorStr), "Failed to make UDP server socket unblocking."); 
            }
        }
        else
        {
            printf(maErrorStr, sizeof(maErrorStr), "Failed to create UDP server socket."); 
        }
        close(tempSocket);*/
    }

    return false;
} // CreateUDPSocket


	

// Receive a data packet. Data is stored in a local static buffer
// Returns number of bytes in received message or -1 if there is an error. 

int CNetwork::Receive(char* rtDataBuff, int nDataBufSize, bool bHeader, int nTimeout, unsigned int *ipAddr)
{

    printf("Here in Receive \n");

    int         nRecved = 0;
    int         nRecvedTotal = 0;
    sockaddr_in source_addr;
    socklen_t        fromlen = sizeof(source_addr);

    fd_set ReadFDs, WriteFDs, ExceptFDs;
    FD_ZERO(&ReadFDs);
    FD_ZERO(&WriteFDs);
    FD_ZERO(&ExceptFDs); 
    
    printf("try to receive shit from...%d\n", mhSocket);
    if (mhSocket != -1)
    {
		//printf("socket is fine FD set...");
        FD_SET(mhSocket, &ReadFDs);
        FD_SET(mhSocket, &ExceptFDs);
    }
    if (mhUDPSocket != -1)
    {
        FD_SET(mhUDPSocket, &ReadFDs);
        FD_SET(mhUDPSocket, &ExceptFDs);
    }
    if (mhUDPBroadcastSocket != -1)
    {
        FD_SET(mhUDPBroadcastSocket, &ReadFDs);
        FD_SET(mhUDPBroadcastSocket, &ExceptFDs);
    }

   timeval *pTimeout;
   timeval  timeout;

    if (nTimeout < 0)
    {
        pTimeout = NULL;
    }
    else
    {
		//printf("setting timeou");
        timeout.tv_sec  = 20000000 / 1000000;
        timeout.tv_usec = 20000000%1000000;//nTimeout % 1000000;
        pTimeout = &timeout;
    }

    printf("Here 1\n");
//--------------------------------------------------------------------------
// This doesn't work - So i Don't no why...- Thereby i just simplified the Communication and it works
//---------------------------------------------------------------------------



    // Wait for activity on the TCP and UDP sockets.
   // int test[nDataBufSize]; 
    // printf("data %d %d%d%d", test[0],test[1], test[2], test[3]);
    //nRecved = recv(mhSocket, rtDataBuff, bHeader ? 8 : nDataBufSize, 0);
    //printf("%d bhead %d ndataSize %d\n", nRecved, bHeader, nDataBufSize);
    //printf("data %d %d%d%d", test[0],test[1], test[2], test[3]);
    //printf ("time out :%d", &timeout.tv_sec);
    //printf ("time out :%d", nTimeout);
    //if ( timeout.tv_sec > 0 )
    //{

    //setsockopt(mhSocket, SOL_SOCKET, SO_RCVTIMEO,
      //         (char*)&(pTimeout), sizeof(pTimeout));
    //}

    //nRecved = recv(mhSocket, rtDataBuff, bHeader ? 8 : nDataBufSize, 0);
    //int nSelectRes = select(0, &ReadFDs, &WriteFDs, &ExceptFDs, pTimeout);
    //printf("%d\n", nSelectRes);
    /*if (nSelectRes > 0)
    {
        if (FD_ISSET(mhSocket, &ExceptFDs))
        {
			printf("General socket error..Except\n");
            // General socket error
            FD_CLR(mhSocket, &ExceptFDs);
            SetErrorString();
            nRecved = SOCKET_ERROR;
        }
        else if (FD_ISSET(mhSocket, &ReadFDs))
        {*/
			//printf("FD_ISSET...Read\n");
            nRecved = recv(mhSocket, rtDataBuff, bHeader ? 8 : nDataBufSize, 0);
            FD_CLR(mhSocket, &ReadFDs);
            //printf("this is the data: %s\n" ,rtDataBuff);
        /*}
        else if (FD_ISSET(mhUDPSocket, &ExceptFDs))
        {
            // General socket error
            
            FD_CLR(mhUDPSocket, &ExceptFDs);
            SetErrorString();
            nRecved = SOCKET_ERROR;
        }
        else if (FD_ISSET(mhUDPSocket, &ReadFDs))
        {
            nRecved = recvfrom(mhUDPSocket, rtDataBuff, nDataBufSize, 0, (sockaddr*)&source_addr, &fromlen);
            FD_CLR(mhUDPSocket, &ReadFDs);
        }
        else if (FD_ISSET(mhUDPBroadcastSocket, &ExceptFDs))
        {
            // General socket error
            FD_CLR(mhUDPBroadcastSocket, &ExceptFDs);
            SetErrorString();
            nRecved = SOCKET_ERROR;
        }
        else if (FD_ISSET(mhUDPBroadcastSocket, &ReadFDs))
        {
            sockaddr_in source_addr;

            nRecved = recvfrom(mhUDPBroadcastSocket, rtDataBuff, nDataBufSize, 0, (sockaddr*)&source_addr, &fromlen);
            FD_CLR(mhUDPBroadcastSocket, &ReadFDs);
            if (ipAddr)
            {
                *ipAddr = source_addr.sin_addr.s_addr;
            }
        }
    }
    else if (nSelectRes == -1)
    {
        nRecved = -1;
        printf("nSelect-1...\n");
    }
    else if (nSelectRes == 0)
    {
        // Select time expired;
        printf("time out...%d\n", nRecved);
        nRecved = 0;
    }
*/

    printf("Here 2\n");
    if (nRecved >= 0)
    {
	printf("i've got data..%d\n", nRecved);
        return nRecved;
    }

    printf("Here 3\n");
    SetErrorString();
    Disconnect();
    printf("fail\n");
    return -1;
} // RecvMessage


bool CNetwork::Send(const char* pSendBuf, int nSize)
{
    int         nSent      = 0;
    int         nTotSent   = 0;

    while (nTotSent < nSize)
    {
        nSent = send(mhSocket, pSendBuf + nTotSent, nSize - nTotSent, 0);
        if (nSent == -1)
        {
            SetErrorString();
            return false;
        }
        nTotSent += nSent;
    }

    return true;
} // Send

//Äquivalent zu iphlpapi.h> fehlt...
/*
bool CNetwork::SendUDPBroadcast(const char* pSendBuf, int nSize, short nPort, unsigned int nFilterAddr /* = 0 */ /*)
{
    bool bBroadCastSent = false;

    if (mhUDPBroadcastSocket != -1)
    {
        IP_ADAPTER_INFO* pAdptInfo  = NULL;
        IP_ADAPTER_INFO* pNextAd    = NULL;
        ULONG ulLen                 = 0;
        DWORD erradapt;
        
        // Find all network interfaces.
        erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);
        if (erradapt == ERROR_BUFFER_OVERFLOW)
        {
            pAdptInfo = (IP_ADAPTER_INFO*)malloc(ulLen);
            erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);      
        }

        if (erradapt == ERROR_SUCCESS)
        {
            sockaddr_in recvAddr;
            recvAddr.sin_family      = AF_INET;
            recvAddr.sin_port        = htons(nPort);
            recvAddr.sin_addr.s_addr = 0xffffffff;

            // Send broadcast on all Ethernet interfaces.
            bool bWaitForResponse = false;
            pNextAd = pAdptInfo;
            while( pNextAd )
            {
                if (pNextAd->Type == MIB_IF_TYPE_ETHERNET)
                {
                    unsigned int nIPaddr = inet_addr(pNextAd->IpAddressList.IpAddress.String);
                    unsigned int nIPmask = inet_addr(pNextAd->IpAddressList.IpMask.String);
                    unsigned int nMaskedLocalIp = nFilterAddr | (~nIPmask);
                    recvAddr.sin_addr.s_addr = nIPaddr | (~nIPmask);
                    if (recvAddr.sin_addr.s_addr != (nFilterAddr | (~nIPmask)))
                    {
                        if (sendto(mhUDPBroadcastSocket, pSendBuf, nSize, 0, (sockaddr*)&recvAddr, sizeof(recvAddr)) == nSize)
                        {
                            bBroadCastSent = true;
                        }
                    }
                }
                pNextAd = pNextAd->Next;
            }
        }
        free(pAdptInfo);      
    }

    return bBroadCastSent;
} // SendUDPBroadcast
*/

void CNetwork::SetErrorString()
{ 
    char *tError = NULL; 
    mnLastError  = GetError(); 
    //DWORD nRet   = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                                // NULL, mnLastError, 0, reinterpret_cast<LPTSTR>(&tError), 0, NULL); 

    printf(maErrorStr, sizeof(maErrorStr), "%s", tError); 

    //LocalFree(tError);
}


char* CNetwork::GetErrorString()
{
    return maErrorStr;
}


int CNetwork::GetError()
{
    return mnLastError;
}

/*
bool CNetwork::IsLocalAddress(unsigned int nAddr)
{
    IP_ADAPTER_INFO* pAdptInfo  = NULL;
    IP_ADAPTER_INFO* pNextAd    = NULL;
    DWORD            erradapt;
    ULONG            ulLen      = 0;
    unsigned int     nAddrTmp   = 0;

    // Find all network interfaces.
    erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);
    if (erradapt == ERROR_BUFFER_OVERFLOW)
    {
        pAdptInfo = (IP_ADAPTER_INFO*)malloc(ulLen);
        erradapt = ::GetAdaptersInfo(pAdptInfo, &ulLen);      
    }

    if (erradapt == ERROR_SUCCESS)
    {
        pNextAd = pAdptInfo;
        while( pNextAd )
        {
            if (pNextAd->Type == MIB_IF_TYPE_ETHERNET)
            {
                // Check if it's a response from a local interface.
                if (inet_addr(pNextAd->IpAddressList.IpAddress.String) == nAddr)
                {
                    return true;
                }
            }
            pNextAd = pNextAd->Next;
        }
    }
    free(pAdptInfo);

    return false;
}*/
