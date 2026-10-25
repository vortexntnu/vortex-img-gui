#ifndef drone_state_h
#define drone_state_h
#include "typedef.h"

// How drone messages actually look
#if 0



typedef struct nav_msgs__msg__Odometry
{
  /// Includes the frame id of the pose parent.
  std_msgs__msg__Header header;
  /// Frame id the pose points to. The twist is in this coordinate frame.
  rosidl_runtime_c__String child_frame_id;
  /// Estimated pose that is typically relative to a fixed world frame.
  geometry_msgs__msg__PoseWithCovariance pose;
  /// Estimated linear and angular velocity relative to child_frame_id.
  geometry_msgs__msg__TwistWithCovariance twist;
} nav_msgs__msg__Odometry;

typedef struct geometry_msgs__msg__TwistWithCovariance
{
  geometry_msgs__msg__Twist twist;
  /// Row-major representation of the 6x6 covariance matrix
  /// The orientation parameters use a fixed-axis representation.
  /// In order, the parameters are:
  /// (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
  double covariance[36];
} geometry_msgs__msg__TwistWithCovariance;

typedef struct geometry_msgs__msg__PoseWithCovarianceStamped
{
  std_msgs__msg__Header header;
  geometry_msgs__msg__PoseWithCovariance pose;
} geometry_msgs__msg__PoseWithCovarianceStamped;
typedef struct geometry_msgs__msg__PoseWithCovariance
{
  geometry_msgs__msg__Pose pose;
  /// Row-major representation of the 6x6 covariance matrix
  /// The orientation parameters use a fixed-axis representation.
  /// In order, the parameters are:
  /// (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
  double covariance[36];
} geometry_msgs__msg__PoseWithCovariance;



  vortex_msgs__msg__OperationMode__AUTONOMOUS = 1
  vortex_msgs__msg__OperationMode__MANUAL = 2
  vortex_msgs__msg__OperationMode__REFERENCE = 3
typedef struct vortex_msgs__msg__OperationMode
{
  uint8_t operation_mode;
} vortex_msgs__msg__OperationMode;
typedef struct vortex_msgs__msg__ReferenceFilterQuat
{
  std_msgs__msg__Header header;
  /// Eta_d
  double x;
  double y;
  double z;
  double qw;
  double qx;
  double qy;
  double qz;
  /// Eta_d_dot
  double x_dot;
  double y_dot;
  double z_dot;
  double roll_dot;
  double pitch_dot;
  double yaw_dot;
} vortex_msgs__msg__ReferenceFilterQuat;

#define ROSIDL_RUNTIME_C__PRIMITIVE_SEQUENCE(STRUCT_NAME, TYPE_NAME)           \
  typedef struct rosidl_runtime_c__##STRUCT_NAME##__Sequence {                 \
    TYPE_NAME *data; /*!< The pointer to an array of STRUCT_NAME */            \
    size_t size;     /*!< The number of valid items in data */                 \
    size_t capacity; /*!< The number of allocated items in data */             \
  } rosidl_runtime_c__##STRUCT_NAME##__Sequence;
typedef struct vortex_msgs__msg__ThrusterForces
{
  std_msgs__msg__Header header;
  rosidl_runtime_c__double__Sequence thrust;
} vortex_msgs__msg__ThrusterForces;
#endif

//=================================================== ANTALL ==========

#define THRUSTER_COUNT 8
#define ELHOUSE_COUNT 2            // Antall elektronikkhus
#define TEMP_SENSORS_PER_ELHOUSE 8 // Sensorer per elhus, til temperaturkart
#define LIGHT_COUNT 2
#define SERVO_COUNT 2

// Én verdi per PCB. Bytt navnene til deres faktiske PCB-er
enum pcb_id {
  PCB_Power,          // Strømfordeling / batteri
  PCB_ThrusterDriver, // ESC-er
  PCB_Main,           // Hoveddatamaskin
  PCB_Sensor,
  PCB_Actuator, // Lys, servoer
  PCB_COUNT     // Må stå sist: antall PCB-er
};

// Lagres som u8 i structen så størrelsen er fast
enum pcb_status {
  PcbStatus_Unknown = 0, // Ikke hørt fra ennå
  PcbStatus_Responding = 1,
  PcbStatus_NotResponding = 2,
  PcbStatus_Error = 3,
};

//=================================================== BYGGEKLOSSER ==========
struct v3 {
  f32 X;
  f32 Y;
  f32 Z;
};

struct quat { // Orientering som kvaternion, samme som ROS
  f32 X;
  f32 Y;
  f32 Z;
  f32 W;
};

//=================================================== GENERELL ==========
// Tilsvarer nav_msgs/Odometry (uten kovarians)
struct odometry {
  v3 Pos; // meter
  quat Orientation;
  v3 LinearVel;  // m/s
  v3 AngularVel; // rad/s
};

// GUI-et sjekker om Sequence øker. Står den stille = mistet kontakt
struct heartbeat {
  u64 TimestampMS; // Dronens klokke
  u32 Sequence;    // +1 for hver melding
};

//=================================================== CONTROL + EMBEDDED
//==========
struct thruster {
  f32 Speed;   // -1.0 til 1.0, settpunkt
  f32 Current; // ampere
  f32 Health;  // 0-100 %
};

struct pcb {
  f32 Voltage;    // volt
  f32 Current;    // ampere
  f32 Power;      // watt
  u32 LastSeenMS; // Tid siden siste svar
  u8 Status;      // pcb_status
};

struct elhouse {
  f32 AmbientTemp;                     // °C
  f32 Temps[TEMP_SENSORS_PER_ELHOUSE]; // °C, til temperaturkart
};

struct battery {
  f32 Voltage; // volt
  f32 Percent; // 0-100 %
};

struct actuators {
  f32 LightPercent[LIGHT_COUNT]; // 0-100 %
  f32 ServoAngle[SERVO_COUNT];   // grader
};

//=================================================== ALT SAMMEN ==========
struct drone_state {
  heartbeat Heartbeat;
  odometry Odom;
  f32 Depth; // meter
  battery Battery;
  thruster Thrusters[THRUSTER_COUNT];
  pcb Pcbs[PCB_COUNT]; // Indekser med pcb_id, f.eks. Pcbs[PCB_Power]
  elhouse Elhouses[ELHOUSE_COUNT];
  actuators Actuators;
  b32 IsRosbagRecording; // Bekreftelse fra dronen
};

#endif
