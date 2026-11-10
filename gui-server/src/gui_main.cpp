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
  gui_ros_topic_subscription Bms0Sub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/bms_0", gui_ros_message_type_bms_status, 0);
  gui_ros_topic_subscription Bms1Sub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/bms_1", gui_ros_message_type_bms_status, 1);
  gui_ros_topic_subscription Motor0Sub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/motorcontroller_0", gui_ros_message_type_motor_controller_status, 0);
  gui_ros_topic_subscription Motor1Sub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/motorcontroller_1", gui_ros_message_type_motor_controller_status, 1);
  gui_ros_topic_subscription ThrusterStatusSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/thruster_status", gui_ros_message_type_thruster_status);
  gui_ros_topic_subscription PcbSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/pcb_status", gui_ros_message_type_pcb_status);
  gui_ros_topic_subscription PowerSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/power_interface", gui_ros_message_type_power_interface);
  gui_ros_topic_subscription HwKillSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/hardware_killswitch", gui_ros_message_type_hardware_killswitch, 0, true);
  gui_ros_topic_subscription TorpedoSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/torpedo", gui_ros_message_type_torpedo);
  gui_ros_topic_subscription TorpedoCountSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/torpedoes_left", gui_ros_message_type_torpedo_count, 0, true);
  gui_ros_topic_subscription DropperSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/dropper", gui_ros_message_type_dropper);
  gui_ros_topic_subscription MarkerCountSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/markers_left", gui_ros_message_type_marker_count, 0, true);
  gui_ros_topic_subscription GripperSub = GUIRosSubscribeToTopic(
      &GUIRosState, "/nautilus/embedded/gripper", gui_ros_message_type_gripper);

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
  while (ListenSocket.State == platform_socket_state_valid) {

    platform_socket AcceptedSocket = PlatformAccept(&ListenSocket);
    while (AcceptedSocket.State == platform_socket_state_valid) {

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
      while (!GUIRosHandleNextMessage(&Bms0Sub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&Bms1Sub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&Motor0Sub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&Motor1Sub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&ThrusterStatusSub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&PcbSub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&PowerSub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&HwKillSub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&TorpedoSub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&TorpedoCountSub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&DropperSub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&MarkerCountSub, &DroneState))
      ;
      while (!GUIRosHandleNextMessage(&GripperSub, &DroneState))
      ;
      PlatformSend(&AcceptedSocket, (u8 *)&DroneState, sizeof(DroneState));
      timespec SleepTime = {};
      // SleepTime.tv_sec = 1;
      SleepTime.tv_nsec = 10000000;
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
