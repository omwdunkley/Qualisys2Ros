#include "RTProtocol.h"
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <string.h>

/**
 Linux (POSIX) implementation of _kbhit().
 Morgan McGuire, morgan@cs.brown.edu
 */
#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

#define QTM_RT_SERVER_BASE_PORT 22222

int main(int argc, char **argv)
{
    CRTProtocol poRTProtocol;
    //HANDLE      hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // By default assume you want to connect to QTM at the same machine - just for testing
    char pServerAddr[32] = "130.75.144.204";

    // Check the command line for the server address
    if (argc > 1)
    {
        strcpy(pServerAddr, argv[1]);
    }

    // The base port (as entered in QTM, TCP/IP port number, in the RT output tab of the workspace options.
    int nBasePort = QTM_RT_SERVER_BASE_PORT; // Use default port if there is no argument
    if (argc > 2)
    {
        nBasePort = strtoul(argv[2], NULL, 10);
    }

    if (poRTProtocol.Connect(pServerAddr, QTM_RT_SERVER_BASE_PORT, 0, 1, 7))
    {
        // Get settings from QTM
        if (poRTProtocol.Read6DOFSettings())
        {
            int nBodies = poRTProtocol.Get6DOFBodyCount();

            printf("There %s %d 6DOF %s\n\n", nBodies == 1 ? "is" : "are", nBodies, nBodies == 1 ? "body" : "bodies");

            CRTProtocol::SPoint sPoint;

            for (int iBody = 0; iBody < nBodies; iBody++)
            {
                printf("Body #%d\n", iBody);
                printf("  Name:  %s\n",   poRTProtocol.Get6DOFBodyName(iBody));
                printf("  Color: %.6X\n", poRTProtocol.Get6DOFBodyColor(iBody));
                for (unsigned int iPoint = 0; iPoint < poRTProtocol.Get6DOFBodyPointCount(iBody); iPoint++)
                {
                    poRTProtocol.Get6DOFBodyPoint(iBody, iPoint, sPoint);
                    printf("  Point: X = %9f  Y = %9f  Z = %9f\n", sPoint.fX, sPoint.fY, sPoint.fZ);
                }
                printf("\n");
            }
        }

        printf("Press any key to start measurement\n");
        getchar();
        system("cls");

        poRTProtocol.StreamFrames(CRTProtocol::RateAllFrames, 0, 0, NULL, CRTProtocol::Component6dEuler);

        CRTPacket::EPacketType eType;
        bool                   bKeyAbort  = false;
        unsigned int           nCount;
        CRTPacket*             pRTPacket;
        float                  fX, fY, fZ, fAng1, fAng2, fAng3;

        while (_kbhit() == 0)
        {
            if (poRTProtocol.ReceiveRTPacket(eType, true))
            {
                switch (eType) 
                {
                    case CRTPacket::PacketError : // sHeader.nType 0 indicates an error
                        fprintf(stderr, "Error when streaming frames: %s\n", poRTProtocol.GetRTPacket()->GetErrorString());
                        break;
                    case CRTPacket::PacketData:         // Data received
                        pRTPacket = poRTProtocol.GetRTPacket();
                        nCount  = pRTPacket->Get6DOFEulerBodyCount();

                        if (nCount > 0)
                        {

                            for (unsigned int i = 0; i < nCount; i++)
                            {
                                char* label = (char*)poRTProtocol.Get6DOFBodyName(i);
                                char  emptyString[] = "";
                                if (label == NULL)
                                {
                                    label = emptyString;
                                }
                                pRTPacket->Get6DOFEulerBody(i, fX, fY, fZ, fAng1, fAng2, fAng3);

                                printf("%15s : X=    %10.4f  Y=     %10.4f  Z=   %10.4f\n", label, fX, fY, fZ);
                                printf("                  Roll= %10.4f  Pitch= %10.4f  Yaw= %10.4f\n\n", fAng1, fAng2, fAng3);
                            }
                            printf("\n");
                        }
                        break;
                    case CRTPacket::PacketNoMoreData :  // No more data
                        break;
                    default:
                        break;
                }
            }
            else
            {
                break;
            }
        }

        getchar(); // Consume key pressed

        poRTProtocol.StreamFramesStop();

        poRTProtocol.Disconnect(); // Disconnect from the server
    }

    return 1;
} // main
