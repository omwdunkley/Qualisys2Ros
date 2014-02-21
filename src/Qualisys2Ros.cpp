//#pragma warning( push )
//#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include "RTProtocol.h"
#pragma warning( pop )

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <math.h>
#include <limits.h>
#include <string>
#include <iomanip>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/TransformStamped.h>

// Including ROS and SFBcomm
#include <ros/ros.h>
#include <sstream>

using namespace std;


// Start the main program
int main(int argc, char* argv[]) {
    const float deg2rad = M_PI/180.;



    string serverAddress = "10.152.185.84"; //"tracking1";		// The address of the computer connected to the Qualisys motion tracking system (ex: "130.75.144.179")
    int basePort = 22222; 					// The base port (as entered in QTM, TCP/IP port number, in the RT output tab of the workspace options

    // Defining global variables
    float x, y, z, roll, pitch, yaw;
    uint bodyCount;
    uint frameNumber;

    CRTPacket*             pRTPacket;
    CRTPacket::EPacketType eType;

    ros::init(argc, argv, "Qualisys2Ros");
    tf::TransformBroadcaster publisher;

    // Defining a protocol that connects to the Qualisys
    CRTProtocol poRTProtocol;

    // Connecting to the server
    ROS_INFO_STREAM("Connecting to the Qualisys Motion Tracking system specified at: " << serverAddress << ":" << basePort);

    if (!poRTProtocol.Connect((char*)serverAddress.data(), basePort, 0, 1, 7)){

        ROS_FATAL_STREAM("Could not find the Qualisys Motion Tracking system at: " << serverAddress << ":" << basePort);
        return 0;
    }

    ROS_INFO_STREAM("Connected to " << serverAddress << ":" << basePort);
    ROS_INFO_STREAM("Entering the measurement streaming loop...");

    int createBodyBufferFlag = 0;
    int createMarkerBufferFlag = 0;
    int printOnce = 0;

    // Infinite Measurement Loop
    while (ros::ok()) {
        pRTPacket = poRTProtocol.GetRTPacket();
        frameNumber  = pRTPacket->GetFrameNumber();

        poRTProtocol.GetCurrentFrame(CRTProtocol::Component6dEuler);

        if (poRTProtocol.ReceiveRTPacket(eType, true)) {
            switch (eType) {
                // Case 1 - sHeader.nType 0 indicates an error
                case CRTPacket::PacketError :
                    ROS_ERROR_STREAM_THROTTLE(1,"Error when streaming frames: " << poRTProtocol.GetRTPacket()->GetErrorString());
                    break;
                case CRTPacket::PacketNoMoreData :  // No more data
                    ROS_WARN_STREAM_THROTTLE(1,"No more data");
                    break;

                // Case 2 - Data received
                case CRTPacket::PacketData:
                    bodyCount  = pRTPacket->Get6DOFEulerBodyCount();
                    if (bodyCount <= 0) {
                        ROS_WARN_THROTTLE(1,"No Bodies Found");
                    } else {
                        for (int i = 0; i < bodyCount; i++) {
                            pRTPacket->Get6DOFEulerBody(i, x, y, z, roll, pitch, yaw);
                            if (isnan(x)||isnan(y)||isnan(z)||isnan(roll)||isnan(pitch)||isnan(yaw)) {
                                ROS_WARN_STREAM_THROTTLE(3, "Rigid-body " << i+1 << "/" << bodyCount << " not detected");
                            } else {
                                // ROTATION: GLOBAL (FIXED) X Y Z (R P Y)
                                stringstream name;
                                name << "Q" << i;
                                //Qualisys sometimes flips 180 degrees around the x axis
                                if (roll>90){                                    
                                    roll -= 180;
                                } else if (roll<-90) {
                                    roll += 180;
                                }
                                publisher.sendTransform(tf::StampedTransform(tf::Transform(tf::createQuaternionFromRPY(roll*deg2rad, pitch*deg2rad, yaw*deg2rad), tf::Vector3(x,y,z)/1000.), ros::Time::now(), "Qualisys", name.str()));
                                ros::spinOnce();
                            }
                        }
                    }
                    break;

                default:
                    ROS_ERROR_THROTTLE(1, "Unknown CRTPacket case");
            }

        }
    }

    ROS_INFO("Shutting down");
    poRTProtocol.StreamFramesStop();
    poRTProtocol.Disconnect();
    return 1;
}
