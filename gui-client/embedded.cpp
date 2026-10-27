#include <imgui.h>    // ImGui::Begin, MenuItem osv.
#include "../lib/typedef.h"
#include "../lib/drone_state.h"

const char* ThrusterStatus(u8 Status) {
    switch (Status) {
        case gui_ThrusterStatus__UNKNOWN: return "Unknown";
        case gui_ThrusterStatus__HEALTHY:     return "Healthy";
        case gui_ThrusterStatus__DEGRADED:  return "Degraded";
        case gui_ThrusterStatus__RPM_FAULT:  return "RPM Fault";
        case gui_ThrusterStatus__ESC_FAULT:  return "ESC Fault";
        case gui_ThrusterStatus__FAILED:  return "Failed";
    }
    return "Ukjent";    // 0 = ingen melding mottatt ennå
}

// Menyen i menylinja. MenuItem med bool* bytter verdien selv og viser hake når vinduet er åpent
void DrawEmbeddedMenu(bool* ElHousesOpen, bool* PcbStatusOpen, bool* ThrustersOpen) {
    if (ImGui::BeginMenu("Embedded")) {
        ImGui::MenuItem("El-Houses",  nullptr, ElHousesOpen);
        ImGui::MenuItem("PCB status", nullptr, PcbStatusOpen);
        ImGui::MenuItem("Thrusters",  nullptr, ThrustersOpen);
        ImGui::EndMenu();   // Bare én EndMenu, og bare når BeginMenu returnerte true
    }
}

// Vinduene. Innholdet kommer når dataene finnes i gui_drone_state
void DrawEmbeddedWindows(gui_drone_state* DroneState, bool* ElHousesOpen, bool* PcbStatusOpen, bool* ThrustersOpen) {
    (void)DroneState;

    if (*ElHousesOpen) {
        ImGui::Begin("El-Houses", ElHousesOpen);    
        ImGui::End();
    }
    if (*PcbStatusOpen) {
        ImGui::Begin("PCB status", PcbStatusOpen);
        ImGui::End();
    }
    if (*ThrustersOpen) {
        ImGui::BeginTable ("Thrusters", 4);
            ImGui::TableSetupColumn("Thruster");
            ImGui::TableSetupColumn("Status");
            ImGui::TableSetupColumn("eRPM Target/Actual");
            ImGui::TableSetupColumn("Current");

            for (s32 I = 0; I < 8; I++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%d", I);
                ImGui::TableNextColumn(); ImGui::Text("%d", ThrusterStatus(u8 Status));
                ImGui::TableNextColumn(); ImGui::Text("%.2f", 0.0f);
                ImGui::TableNextColumn(); ImGui::Text("%.2f", 0.0f);
            
            ImGui::EndTable();
        }
        
        
    }
}
