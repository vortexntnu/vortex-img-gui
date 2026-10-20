#include <rcl/context.h>
#include <rcl/error_handling.h>
#include <rcl/graph.h>
#include <rcl/rcl.h>
#include <rcl/subscription.h>
#include <rosidl_runtime_c/message_type_support_struct.h>

#include <rmw/names_and_types.h>

#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/string.h>

#include <geometry_msgs/msg/pose_with_covariance_stamped.h>
#include <nav_msgs/msg/odometry.h>

#include <vortex_msgs/msg/operation_mode.h>
#include <vortex_msgs/msg/reference_filter_quat.h>
#include <vortex_msgs/msg/thruster_forces.h>

#include "util.h"

#define GUI_NODE_NAME "vortex_gui_node"

struct gui_ros_state {
  rcl_allocator_t RCLAllocator;
  rcl_init_options_t RCLInitOptions;
  rcl_context_t RCLContext;
  rcl_node_options_t RCLNodeOptions;

  rcl_node_t RCLNode;

  rcl_subscription_options_t RCLDefaultSubscriptionOptions;

  bool IsValid;
};

enum {
  gui_ros_message_type_invalid = 0,
  gui_ros_message_type_bool,
  gui_ros_message_type_string,
  gui_ros_message_type_odometry,

  gui_ros_message_type_pose,
  gui_ros_message_type_quat,
  gui_ros_message_type_operationmode,
  gui_ros_message_type_thrusterforces,
};

struct gui_ros_topic_subscription {
  rcl_subscription_t RCLSubscription;
  bool IsValid;

  int MessageType;
  char TopicName[128];
};
void InitGUIRosState(gui_ros_state *State, int ArgC, char **ArgV) {
  // TODO(Boje):Verify that State is unitialized
  int Error = 0;

  *State = gui_ros_state{};
  State->RCLAllocator = rcl_get_default_allocator();
  rcl_ret_t RCLReturnCode =
      rcl_init_options_init(&State->RCLInitOptions, State->RCLAllocator);
  if (RCLReturnCode != RCL_RET_OK) {
    Error++;
  }
  RCLReturnCode =
      rcl_init(ArgC, ArgV, &State->RCLInitOptions, &State->RCLContext);
  if (RCLReturnCode != RCL_RET_OK) {
    Error++;
  }
  State->RCLNodeOptions = rcl_node_get_default_options();

  RCLReturnCode = rcl_node_init(&State->RCLNode, GUI_NODE_NAME, "",
                                &State->RCLContext, &State->RCLNodeOptions);
  if (RCLReturnCode != RCL_RET_OK) {
    Error++;
  }
  State->RCLDefaultSubscriptionOptions = rcl_subscription_get_default_options();
  State->RCLDefaultSubscriptionOptions.qos = rmw_qos_profile_sensor_data;
  if (Error) {
    *State = gui_ros_state{};
    LogError("Failed to initialize ros state");
  }
}

void FreeGUIRosState(gui_ros_state *State) {
  rcl_ret_t RCLReturnCode = rcl_node_fini(&State->RCLNode);
  if (RCLReturnCode != RCL_RET_OK) {
    LogError("RCL error during rcl_node_fini\n");
    printf("Code: %d\n", RCLReturnCode);
  }
  RCLReturnCode = rcl_shutdown(&State->RCLContext);
  if (RCLReturnCode != RCL_RET_OK) {
    LogError("RCL error during rcl_context_fini\n");
  }
  RCLReturnCode = rcl_context_fini(&State->RCLContext);
  if (RCLReturnCode != RCL_RET_OK) {
    LogError("RCL error during rcl_context_fini\n");
  }
  RCLReturnCode = rcl_init_options_fini(&State->RCLInitOptions);
  if (RCLReturnCode != RCL_RET_OK) {
    LogError("RCL error during rcl_init_options_fini\n");
  }
  RCLReturnCode =
      rcl_subscription_options_fini(&State->RCLDefaultSubscriptionOptions);
  if (RCLReturnCode != RCL_RET_OK) {
    LogError("RCL error during rcl_subscription_options_fini\n");
  }
}

gui_ros_topic_subscription GUIRosSubscribeToTopic(gui_ros_state *State,
                                                  const char *Topic,
                                                  int MessageType) {
  gui_ros_topic_subscription Result = {};

  if (CountChar(Topic) > ArrayCount(Result.TopicName) + 1) {
    LogError("Topic name is too long");
    return Result;
  }
  Result.MessageType = MessageType;
  FromCStrCopyToDest(Topic, Result.TopicName, ArrayCount(Result.TopicName) - 1);
  const rosidl_message_type_support_t *Ts = {};
  switch (MessageType) {
  case gui_ros_message_type_bool: {
    Ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool);
  } break;
  case gui_ros_message_type_string: {
    Ts = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  } break;
  case gui_ros_message_type_odometry: {
    Ts = ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry);
  } break;

  case gui_ros_message_type_pose: {
    Ts = ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg,
                                     PoseWithCovarianceStamped);
  } break;
  case gui_ros_message_type_quat: {
    Ts = ROSIDL_GET_MSG_TYPE_SUPPORT(vortex_msgs, msg, ReferenceFilterQuat);
  } break;
  case gui_ros_message_type_operationmode: {
    Ts = ROSIDL_GET_MSG_TYPE_SUPPORT(vortex_msgs, msg, OperationMode);
  } break;
  case gui_ros_message_type_thrusterforces: {
    Ts = ROSIDL_GET_MSG_TYPE_SUPPORT(vortex_msgs, msg, ThrusterForces);
  } break;
  default: {
    LogError("Unrecognized message type");
  } break;
  }

  rcl_ret_t RCLReturnCode = rcl_subscription_init(
      &Result.RCLSubscription, &State->RCLNode, Ts, Result.TopicName,
      &State->RCLDefaultSubscriptionOptions);
  if (RCLReturnCode == RCL_RET_OK) {
    Result.IsValid = 1;
  } else {
    LogError("RCL error during rcl_subscription_init\n");
    printf("%s\n", rcl_get_error_string().str);
    printf(" Code: %d\n", RCLReturnCode);
  }

  return Result;
}

// This function returns 0 when a message was taken and 1 when the queue is
// empty
int GUIRosHandleNextMessage(gui_ros_topic_subscription *Sub) {
  rmw_message_info_t RCLMessageInfo = {};
  int Result = -1;

  rcl_ret_t RCLReturnCode = 0;
  switch (Sub->MessageType) {
  case gui_ros_message_type_bool: {
    std_msgs__msg__Bool Msg;
    std_msgs__msg__Bool__init(&Msg);

    RCLReturnCode =
        rcl_take(&Sub->RCLSubscription, &Msg, &RCLMessageInfo, NULL);
    if (RCLReturnCode == RCL_RET_OK) {
      printf("BoolMessage: %d \n", Msg.data);
    }
    std_msgs__msg__Bool__fini(&Msg);
  } break;
  case gui_ros_message_type_string: {
    std_msgs__msg__String Msg;
    std_msgs__msg__String__init(&Msg);

    RCLReturnCode =
        rcl_take(&Sub->RCLSubscription, &Msg, &RCLMessageInfo, NULL);
    std_msgs__msg__String__fini(&Msg);
  } break;
  case gui_ros_message_type_odometry: {
    nav_msgs__msg__Odometry Msg;
    nav_msgs__msg__Odometry__init(&Msg);

    RCLReturnCode =
        rcl_take(&Sub->RCLSubscription, &Msg, &RCLMessageInfo, NULL);

    nav_msgs__msg__Odometry__fini(&Msg);
  } break;
  case gui_ros_message_type_pose: {
    geometry_msgs__msg__PoseWithCovarianceStamped Msg;
    geometry_msgs__msg__PoseWithCovarianceStamped__init(&Msg);

    RCLReturnCode =
        rcl_take(&Sub->RCLSubscription, &Msg, &RCLMessageInfo, NULL);
    geometry_msgs__msg__PoseWithCovarianceStamped__fini(&Msg);
  } break;
  case gui_ros_message_type_quat: {
    vortex_msgs__msg__ReferenceFilterQuat Msg;
    vortex_msgs__msg__ReferenceFilterQuat__init(&Msg);

    RCLReturnCode =
        rcl_take(&Sub->RCLSubscription, &Msg, &RCLMessageInfo, NULL);
    vortex_msgs__msg__ReferenceFilterQuat__fini(&Msg);
  } break;
  case gui_ros_message_type_operationmode: {
    vortex_msgs__msg__OperationMode Msg;
    vortex_msgs__msg__OperationMode__init(&Msg);

    RCLReturnCode =
        rcl_take(&Sub->RCLSubscription, &Msg, &RCLMessageInfo, NULL);
    vortex_msgs__msg__OperationMode__fini(&Msg);
  } break;
  case gui_ros_message_type_thrusterforces: {
    vortex_msgs__msg__ThrusterForces Msg;
    vortex_msgs__msg__ThrusterForces__init(&Msg);

    RCLReturnCode =
        rcl_take(&Sub->RCLSubscription, &Msg, &RCLMessageInfo, NULL);
    vortex_msgs__msg__ThrusterForces__init(&Msg);
  } break;
  default: {
    LogError("Invalid message type");
  } break;
  }

  if (RCLReturnCode == RCL_RET_OK) {
    Result = 0;
    printf("GOT MESSAGE! on %s\n", Sub->TopicName);
  }
  if (RCLReturnCode != RCL_RET_OK &&
      RCLReturnCode != RCL_RET_SUBSCRIPTION_TAKE_FAILED) {
    LogError("RCL error during rcl_take\n");
    LogError(rcl_get_error_string().str);
  }
  if (RCLReturnCode == RCL_RET_SUBSCRIPTION_TAKE_FAILED) {
    Result = 1;
  }
  return Result;
}

// Unused functions
void GUIRosPrintAllTopics(gui_ros_state *State) {
  rcl_names_and_types_t RCLTopics = rcl_get_zero_initialized_names_and_types();

  rcl_ret_t RCLReturnCode = rcl_get_topic_names_and_types(
      &State->RCLNode, &State->RCLAllocator, false, &RCLTopics);
  if (RCLReturnCode != RCL_RET_OK) {
    LogError("RCL error during rcl_subscription_init\n");

    printf("Code: %d\n", RCLReturnCode);
  }
  for (size_t I = 0; I < RCLTopics.names.size; I++) {
    printf("Topic Name: %s", RCLTopics.names.data[I]);
    for (size_t II = 0; II < RCLTopics.types[I].size; II++) {
      printf(" | DataType: %s", RCLTopics.types[I].data[II]);
    }
    printf("\n");
  }
  RCLReturnCode = rcl_names_and_types_fini(&RCLTopics);
  if (RCLReturnCode != RCL_RET_OK) {
    LogError("RCL error during rcl_names_and_types_fini\n");

    printf("Code: %d\n", RCLReturnCode);
  }
}
