#include <math.h>     // atan2, asin
#include <imgui.h>    // ImGui::Begin, Text, ImVec4 osv.
#include "../lib/typedef.h"
#include "../lib/drone_state.h"

//=================================================== GENERELL-SIDEN ==========
// Viser pose, killswitch og operation mode.
//   KillSwitch    <- /nautilus/killswitch      (std_msgs/Bool)
//   OperationMode <- /nautilus/operation_mode  (vortex_msgs/OperationMode)
//   Pose          <- /nautilus/pose            (geometry_msgs/PoseWithCovarianceStamped)

#define Pi64 3.14159265358979323846

struct euler_angles {   // grader
    f64 Roll;
    f64 Pitch;
    f64 Yaw;
};

// Kvaternion -> roll/pitch/yaw (ZYX-rekkefølge, samme som tf2 i ROS)
euler_angles QuatToEuler(v4 Q) {
    euler_angles Result = {};

    f64 SinPitch = 2.0*(Q.W*Q.Y - Q.Z*Q.X);
    if (SinPitch >  1.0) SinPitch =  1.0;    // Avrundingsfeil kan gi litt over 1, det tåler ikke asin
    if (SinPitch < -1.0) SinPitch = -1.0;

    Result.Roll  = atan2(2.0*(Q.W*Q.X + Q.Y*Q.Z), 1.0 - 2.0*(Q.X*Q.X + Q.Y*Q.Y)) * 180.0/Pi64;
    Result.Pitch = asin(SinPitch)                                                * 180.0/Pi64;
    Result.Yaw   = atan2(2.0*(Q.W*Q.Z + Q.X*Q.Y), 1.0 - 2.0*(Q.Y*Q.Y + Q.Z*Q.Z)) * 180.0/Pi64;
    return Result;
}

// Navnet til en verdi fra gui_OperationMode-enumen i drone_state.h
const char* OperationModeName(u8 OperationMode) {
    switch (OperationMode) {
        case gui_OperationMode__AUTONOMOUS: return "Autonomous";
        case gui_OperationMode__MANUAL:     return "Manual";
        case gui_OperationMode__REFERENCE:  return "Reference";
    }
    return "Ukjent";    // 0 = ingen melding mottatt ennå
}

// IsOpen kommer fra Visible["general"] i main.cpp, så den lagres i layoutene
void DrawGeneralPage(gui_drone_state* DroneState, bool* IsOpen) {
    if (!*IsOpen)
        return;
    ImGui::Begin("Generell", IsOpen);   // X i vinduet setter *IsOpen = false


    // true = dronen er stoppet
    ImGui::Text("Killswitch:");
    ImGui::SameLine();
        ImVec4 KillSwitchColor;
    if (DroneState->KillSwitch) {                          
        KillSwitchColor = ImVec4(0.0f, 0.5f, 0.0f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Button,KillSwitchColor);     
        if (ImGui::Button("ON")){
            SetKillSwitch(true);
        }
        ImGui::PopStyleColor(); 
          
        ImGui::SameLine();
        if (ImGui::Button("OFF")){
            SetKillSwitch(false);
        }
    }
    else {
        KillSwitchColor = ImVec4(0.5f, 0.0f, 0.0f, 1.0f);     
        if (ImGui::Button("ON")){
        SetKillSwitch(true);
        }
        ImGui::PushStyleColor(ImGuiCol_Button,KillSwitchColor);  
        ImGui::SameLine();
        if (ImGui::Button("OFF")){
           SetKillSwitch(false);
        } 
        ImGui::PopStyleColor(); 
    }
    
    
    ImGui::SameLine();


    ImGui::Text("Operation mode:");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##OperationMode", OperationModeName(DroneState->OperationMode))) {
        for (u8 Mode = gui_OperationMode__AUTONOMOUS; Mode <= gui_OperationMode__REFERENCE; Mode++) {
            if (ImGui::Selectable(OperationModeName(Mode), DroneState->OperationMode == Mode)) {
                SetOperationMode(Mode);            
            }
        }
        ImGui::EndCombo();
    }


    ImGui::SeparatorText("Pose");
    gui_pose* Pose = &DroneState->Pose;
    euler_angles Angles = QuatToEuler(Pose->OrientationQuat);
    ImGui::Text("Posisjon [m]:      X %7.2f   Y %7.2f   Z %7.2f", Pose->Position.X, Pose->Position.Y, Pose->Position.Z);
    ImGui::Text("Orientering [deg]: R %7.2f   P %7.2f   Y %7.2f", Angles.Roll, Angles.Pitch, Angles.Yaw);

    ImGui::End();
}
