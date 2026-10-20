#include <vector>
#include <string>
#include <imgui.h>

std::vector<std::string> guiConsoleLines;//lager liste (vektor) som tar vare på "string tall" til listen guiConsoleLines
int consoleNumberLine = 0;

void toGUIConsole(std::string text){
    consoleNumberLine += 1;
    guiConsoleLines.push_back(std::to_string(consoleNumberLine) + ": " + text);
}

void drawConsole(bool* open) {
    if (ImGui::Begin("GUI-Console", open)) {
        for (const std::string& line : guiConsoleLines) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", line.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f); // auto-scroll nedover
    }
    ImGui::End();   // alltid, også når Begin returnerer false
}