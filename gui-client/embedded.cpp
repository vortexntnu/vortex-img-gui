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

// Bakgrunnsfarge for statuscella. Mørke nok til at hvit tekst er lett å lese
ImVec4 ThrusterStatusColor(u8 Status) {
    switch (Status) {
        case gui_ThrusterStatus__HEALTHY:  return ImVec4(0.0f, 0.5f,  0.0f, 1.0f);   // grønn
        case gui_ThrusterStatus__DEGRADED: return ImVec4(0.7f, 0.35f, 0.0f, 1.0f);   // oransje
        case gui_ThrusterStatus__UNKNOWN:  return ImVec4(0.0f, 0.0f,  0.0f, 1.0f);   // svart
    }
    return ImVec4(0.7f, 0.1f, 0.1f, 1.0f);          
    // rød: alle feil
}

        


// Menyen i menylinja. MenuItem med bool* bytter verdien selv og viser hake når vinduet er åpent
void DrawEmbeddedMenu(bool* ElHousesOpen, bool* PcbStatusOpen, bool* ThrustersOpen, bool* ActuatorStatusOpen) {
    if (ImGui::BeginMenu("Embedded")) {
        ImGui::MenuItem("El-Houses",  nullptr, ElHousesOpen);
        ImGui::MenuItem("PCB status", nullptr, PcbStatusOpen);
        ImGui::MenuItem("Thrusters",  nullptr, ThrustersOpen);
        ImGui::MenuItem("Actuators",  nullptr, ActuatorStatusOpen);
        ImGui::EndMenu();   // Bare én EndMenu, og bare når BeginMenu returnerte true
    }
}

// Vinduene. Innholdet kommer når dataene finnes i gui_drone_state
void DrawEmbeddedWindows(gui_drone_state* DroneState, bool* ElHousesOpen, bool* PcbStatusOpen, bool* ThrustersOpen, bool* ActuatorStatusOpen) {
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
        ImGui::Begin("Thrusters", ThrustersOpen);          

    if (ImGui::BeginTable("ThrusterTabell", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Thruster");
        ImGui::TableSetupColumn("Status");
        ImGui::TableSetupColumn("eRPM Target/Actual");
        ImGui::TableSetupColumn("Current [A]");
        ImGui::TableHeadersRow();                      

        for (s32 I = 0; I < 8; I++) {
            u8 Status = gui_ThrusterStatus__UNKNOWN;  
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%d", I);
            ImGui::TableNextColumn();
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ThrusterStatusColor(Status)));   // bare denne cella
            ImGui::Text("%s", ThrusterStatus(Status));
            ImGui::TableNextColumn(); ImGui::Text("%d / %d", 0, 0);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", 0.0f);
        }                                              

        ImGui::EndTable();                             
    }

    if (*ActuatorStatusOpen) {
        ImGui::Begin("Actuators", ActuatorStatusOpen);
        ImGui::End();
    }

    ImGui::End();
    }
}
