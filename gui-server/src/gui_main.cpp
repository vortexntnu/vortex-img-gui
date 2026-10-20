#include "gui_ros2.cpp"

#include <time.h>

int main(int ArgC, char **ArgV) {
  printf("Printing line\n");

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
  GUIRosHandleNextMessage(&KillSwitchSub);
  for (int I = 0; I < 50000; I++) {

    while (!GUIRosHandleNextMessage(&KillSwitchSub))
      ;

    while (!GUIRosHandleNextMessage(&OdomSub))
      ;

    while (!GUIRosHandleNextMessage(&PoseSub))
      ;

    while (!GUIRosHandleNextMessage(&QuatSub))
      ;

    while (!GUIRosHandleNextMessage(&OpmodeSub))
      ;

    while (!GUIRosHandleNextMessage(&ThrusterSub))
      ;
    //  GUIRosTakeAndPrintFromSubscription(&OdomSub);

    // GUIRosTakeAndPrintFromSubscription(&PoseSub);
    // GUIRosTakeAndPrintFromSubscription(&QuatSub);
    // GUIRosTakeAndPrintFromSubscription(&OpmodeSub);
    // GUIRosTakeAndPrintFromSubscription(&ThrusterSub);

    timespec SleepTime = {};
    // SleepTime.tv_sec = 2;
    SleepTime.tv_nsec = 10000;
    nanosleep(&SleepTime, NULL);
  }
  for (int I = 0; I < 0; I++) {
    GUIRosPrintAllTopics(&GUIRosState);

    timespec SleepTime = {};
    SleepTime.tv_sec = 3;
    nanosleep(&SleepTime, NULL);
  }

  FreeGUIRosState(&GUIRosState);

  return 0;
}
