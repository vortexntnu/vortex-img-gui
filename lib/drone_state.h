#ifndef drone_state_h
#define drone_state_h
#include "typedef.h"

struct v3 {
  f64 X;
  f64 Y;
  f64 Z;
};

struct v4 {
  f64 X;
  f64 Y;
  f64 Z;
  f64 W;
};

struct gui_pose {
  v3 Position;
  v4 OrientationQuat;
  f64 Convariance[36];
};
struct gui_twist {
  v3 Linear;
  v3 Angluar;
  f64 Convariance[36];
};
struct gui_odom {
  gui_pose Pose;
  gui_twist Twist;
};
enum {
  gui_OperationMode__AUTONOMOUS = 1,
  gui_OperationMode__MANUAL = 2,
  gui_OperationMode__REFERENCE = 3,
};

enum {
  gui_ThrusterStatus__UNKNOWN = 0,
  gui_ThrusterStatus__HEALTHY = 1,
  gui_ThrusterStatus__DEGRADED = 2,
  gui_ThrusterStatus__RPM_FAULT = 3,
  gui_ThrusterStatus__ESC_FAULT = 4,
  gui_ThrusterStatus__FAILED = 5,

};

struct gui_referencefilter_quat {
  f64 x;
  f64 y;
  f64 z;
  f64 qw;
  f64 qx;
  f64 qy;
  f64 qz;
  /// Eta_d_dot
  f64 x_dot;
  f64 y_dot;
  f64 z_dot;
  f64 roll_dot;
  f64 pitch_dot;
  f64 yaw_dot;
};

struct gui_drone_state {
  u8 KillSwitch;    // This is a boolean value
  u8 OperationMode; // See enum above
  gui_odom Odometry;
  gui_pose Pose;
  gui_referencefilter_quat ReferenceFilterQuat;
};
// How drone messages actually look

#if 0
e ROSIDL_RUNTIME_C__PRIMITIVE_SEQUENCE(STRUCT_NAME, TYPE_NAME)           \
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
#if 0
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

//=================================================== GENERELL ==========
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
#endif