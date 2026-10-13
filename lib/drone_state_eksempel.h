#ifndef drone_state_eksempel_h
#define drone_state_eksempel_h
#include "typedef.h"

// EKSEMPEL basert på "Vortex GUI need/wants list".
// Alt som sendes fra dronen til GUI-et hvert tick. Bare faste typer (ingen pekere/std::string),
// fordi hele structen sendes som rå bytes. Server og klient må bygges med samme versjon.
//
// Ikke med her (hører hjemme et annet sted):
//   - Kamerabilde og debug-kamerabilde: for stort, må sendes som egen strøm
//   - Valg av debug-topic og start/stopp av rosbag: kommandoer fra GUI til drone (egen drone_command-struct)
//   - Strømflyt-diagram mellom PCB-er: tegnes i GUI-et ut fra strøm/effekt per PCB nedenfor

//=================================================== ANTALL ==========
// Bytt til riktige tall for dronen
#define THRUSTER_COUNT            8
#define ELHOUSE_COUNT             2   // Antall elektronikkhus
#define TEMP_SENSORS_PER_ELHOUSE  8   // Sensorer per elhus, til temperaturkart
#define LIGHT_COUNT               2
#define SERVO_COUNT               2

// Én verdi per PCB. Bytt navnene til deres faktiske PCB-er
enum pcb_id {
	PCB_Power,          // Strømfordeling / batteri
	PCB_ThrusterDriver, // ESC-er
	PCB_Main,           // Hoveddatamaskin
	PCB_Sensor,
	PCB_Actuator,       // Lys, servoer
	PCB_COUNT           // Må stå sist: antall PCB-er
};

// Lagres som u8 i structen så størrelsen er fast
enum pcb_status {
	PcbStatus_Unknown       = 0, // Ikke hørt fra ennå
	PcbStatus_Responding    = 1,
	PcbStatus_NotResponding = 2,
	PcbStatus_Error         = 3,
};

//=================================================== BYGGEKLOSSER ==========
struct v3 {
	f32 X;
	f32 Y;
	f32 Z;
};

struct quat {        // Orientering som kvaternion, samme som ROS
	f32 X;
	f32 Y;
	f32 Z;
	f32 W;
};

//=================================================== GENERELL ==========
// Tilsvarer nav_msgs/Odometry (uten kovarians)
struct odometry {
	v3   Pos;            // meter
	quat Orientation;
	v3   LinearVel;      // m/s
	v3   AngularVel;     // rad/s
};

// GUI-et sjekker om Sequence øker. Står den stille = mistet kontakt
struct heartbeat {
	u64 TimestampMS;     // Dronens klokke
	u32 Sequence;        // +1 for hver melding
};

//=================================================== CONTROL + EMBEDDED ==========
struct thruster {
	f32 Speed;           // -1.0 til 1.0, settpunkt
	f32 Current;         // ampere
	f32 Health;          // 0-100 %
};

struct pcb {
	f32 Voltage;         // volt
	f32 Current;         // ampere
	f32 Power;           // watt
	u32 LastSeenMS;      // Tid siden siste svar
	u8  Status;          // pcb_status
};

struct elhouse {
	f32 AmbientTemp;                          // °C
	f32 Temps[TEMP_SENSORS_PER_ELHOUSE];      // °C, til temperaturkart
};

struct battery {
	f32 Voltage;         // volt
	f32 Percent;         // 0-100 %
};

struct actuators {
	f32 LightPercent[LIGHT_COUNT];   // 0-100 %
	f32 ServoAngle[SERVO_COUNT];     // grader
};

//=================================================== ALT SAMMEN ==========
struct drone_state {
	heartbeat Heartbeat;
	odometry  Odom;
	f32       Depth;                       // meter
	battery   Battery;
	thruster  Thrusters[THRUSTER_COUNT];
	pcb       Pcbs[PCB_COUNT];             // Indekser med pcb_id, f.eks. Pcbs[PCB_Power]
	elhouse   Elhouses[ELHOUSE_COUNT];
	actuators Actuators;
	b32       IsRosbagRecording;           // Bekreftelse fra dronen
};

#endif
