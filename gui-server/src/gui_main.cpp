#define BUILD_WITH_ROS2
#include "gui_ros2.cpp"

#include <time.h>

#include "../../lib/drone_state.h"
#define PLATFORM_LINUX
#include "../../lib/platform.h"

#include <math.h>
int main(int ArgC, char **ArgV) {
  printf("Printing line\n");

  gui_drone_state DroneState = {};

  for (int I = 0; I < ArgC; I++) {
    printf("Arg[%d]: %s\n", I, ArgV[I]);
  }

  gui_ros_state GUIRosState = {};
  InitGUIRosState(&GUIRosState, ArgC, ArgV);
  printf("Subscribing to ros topic\n");

  gui_ros_topic_subscription KillSwitchSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/killswitch", gui_ros_message_type_bool);
  gui_ros_topic_subscription OdomSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/odom", gui_ros_message_type_odometry);
  gui_ros_topic_subscription PoseSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/pose", gui_ros_message_type_pose);
  gui_ros_topic_subscription QuatSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/guidance/dp_quat", gui_ros_message_type_quat);
  gui_ros_topic_subscription OpmodeSub =
      GUIRosSubscribeToTopic(&GUIRosState, "/nautilus/operation_mode",
                             gui_ros_message_type_operationmode);
  gui_ros_topic_subscription ThrusterSub =
      GUIRosSubscribeToTopic(&GUIRosState, "/nautilus/thruster_forces",
                             gui_ros_message_type_thrusterforces);

#if 0

#endif

  PlatformInitializeNetworkLib();

  char *TestMessage = "TestMessage";
  u8 RecvBuffer[1024] = {};

  platform_ip ListenAddress = PlatformIP("127.0.0.1");

  s32 MS = 0;
  s32 SleepTime = 5;
  platform_socket ListenSocket =
      PlatformBindAddressAndListen(ListenAddress, 2100);
  while (ListenSocket.IsValid) {

    platform_socket AcceptedSocket = PlatformAccept(&ListenSocket);
    while (AcceptedSocket.IsValid) {

      while (!GUIRosHandleNextMessage(&KillSwitchSub, &DroneState))
        ;
      while (!GUIRosHandleNextMessage(&OdomSub, &DroneState))
        ;
      while (!GUIRosHandleNextMessage(&PoseSub, &DroneState))
        ;
      while (!GUIRosHandleNextMessage(&OpmodeSub, &DroneState))
        ;
      while (!GUIRosHandleNextMessage(&ThrusterSub, &DroneState))
        ;
      while (!GUIRosHandleNextMessage(&QuatSub, &DroneState))
        ;
      PlatformSend(&AcceptedSocket, (u8 *)&DroneState, sizeof(DroneState));
      timespec SleepTime = {};
      SleepTime.tv_nsec = 100000;
      nanosleep(&SleepTime, NULL);
    }
  }
#if 0
  for (int I = 0; I < 0; I++) {
    GUIRosPrintAllTopics(&GUIRosState);

    timespec SleepTime = {};
    SleepTime.tv_sec = 3;
    nanosleep(&SleepTime, NULL);
  }
#endif

  FreeGUIRosState(&GUIRosState);
  PlatformFreeNetworkLib();
  return 0;
}
