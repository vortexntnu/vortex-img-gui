#include <vector>
#include <string>
#include <imgui.h>


//#include "console.cpp"

enum PoseField{ //Lager binær kode til hver mode for å dele inn
    F_X   = 1<<0,
    F_Y   = 1<<1,
    F_Z   = 1<<2,
    F_ROLL= 1<<3,
    F_PITCH=1<<4,
    F_YAW = 1<<5,
    F_POS = F_X | F_Y | F_Z,
    F_ORI = F_ROLL | F_PITCH | F_YAW,
};


void drawFull_Pose(bool* open) {
    if (ImGui::Begin("Full_Pose", open, ImGuiWindowFlags_MenuBar)) {

        static int mode = 0;
        static const char* modes[] = { //indeks til modes sier hvilken mode som er aktiv
            "FULL_POSE = 0", "ONLY_POSITION = 1", "FORWARD_HEADING = 2",
            "ONLY_ORIENTATION = 3", "POSITION_AND_YAW = 4", "XY_AND_YAW = 5",
            "XY_FORWARD_DIR = 6", "LEVEL_ORIENTATION = 7", "ONLY_Z = 8",
            "POS_Z_LEVEL_ORIENTATION = 9"
        };

        //editable[] indeks forteller hvilken verdi som skal kunne endres. (åpenbart)
        static const int editable[] = {
            F_POS | F_ORI,          // 0 FULL_POSE
            F_POS,                  // 1 ONLY_POSITION
            F_POS,                  // 2 FORWARD_HEADING
            F_ORI,                  // 3 ONLY_ORIENTATION
            F_POS | F_YAW,          // 4 POSITION_AND_YAW
            F_X | F_Y | F_YAW,      // 5 XY_AND_YAW
            F_X | F_Y,              // 6 XY_FORWARD_DIR
            F_YAW,                  // 7 LEVEL_ORIENTATION
            F_Z,                    // 8 ONLY_Z
            F_Z,                    // 9 POS_Z_LEVEL_ORIENTATION
        };

        char menuLabel[96];
        snprintf(menuLabel, sizeof(menuLabel), "WaypointMode.msg = %s###wpmenu", modes[mode]);


        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu(menuLabel)){ //Meny for å velge modus
                for (int i = 0; i < IM_ARRAYSIZE(modes); i++) {
                    if (ImGui::MenuItem(modes[i], nullptr, mode == i))
                        mode = i;
                }
                ImGui::EndMenu();      // bare når menyen er åpen
            }
            ImGui::EndMenuBar();
        }
    
        
        static float fpX, fpY, fpZ, fpR, fpP, fpYaw, fpConvth;
        const int active = editable[mode];

        //Lambda (mini funksjon), [&] gir tilgang til eksterne variabler (i full_pose.cpp) og endrer på orginale, ikke en kopi
        auto field = [&](int bit, const char* label, float* v){ //bit viser modus, label er navn på hva som endres, v er variabelen
            ImGui::BeginDisabled(!(active & bit));
            ImGui::InputFloat(label, v);
            ImGui::EndDisabled();
        };


        ImGui::PushItemWidth(120);
    
        field(F_X,"X   ", &fpX);   ImGui::SameLine();
        field(F_Y,"Y    ", &fpY);   ImGui::SameLine();
        field(F_Z,"Z  ", &fpZ);
        
        ImGui::NewLine();
        
        field(F_ROLL,"Roll", &fpR);  ImGui::SameLine();
        field(F_PITCH,"Pitch", &fpP); ImGui::SameLine();
        field(F_YAW,"Yaw", &fpYaw);
        

        ImGui::NewLine();
        ImGui::InputFloat("Convergance Threshold",&fpConvth);
        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
        if (ImGui::Button("Submit")) {
            toGUIConsole("Full_Pose: Submit pressed.");
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
        if (ImGui::Button("Cancel")) {
            toGUIConsole("Full_Pose: Cancel pressed.");
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.8f, 1.0f));
        if (ImGui::Button("Reset Odom")) {
            toGUIConsole("Full_Pose: Reset Odom pressed.");
        }
        ImGui::PopStyleColor();

    }
    ImGui::End();   // alltid, også når Begin returnerer false
}