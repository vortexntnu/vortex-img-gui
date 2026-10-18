#include <imgui.h>
#include <imgui_internal.h>   // ImGuiSettingsHandler, ImHashStr
#include <SDL3/SDL.h>
#include <stdio.h>    // snprintf
#include <stdlib.h>   // atoi
#include <string.h>   // strchr
#include <map>        // std::map (dictionary)
#include <string>     // std::string
#include "../lib/typedef.h"


// Alt som trengs for å laste og lagre layouts
struct layout_state {
    std::map<std::string, bool>* Visible;   // Hvilke vinduer som er åpne (eies av main)
    const char* ExeFolder;                   // f.eks. "...\gui-client\BuildOutput\"
    std::string PendingLoad;                 // Full sti til layout som skal lastes. Tom = ingenting
    bool PendingSave = false;                // Settes når Enter trykkes i "Lagre layout som"
    char NewName[64] = "";                   // Navnet som skrives i "Lagre layout som"
};


//=================================================== LAYOUT: HVILKE VINDUER ER ÅPNE ==========
// Lagres som en egen seksjon i layout-filene:
//   [Vortex][Visibility]
//   video=1
// Kalles når ImGui finner [Vortex][Visibility] i fila. Må returnere noe som ikke er nullptr.
static void* VortexReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char*)
{
    return (void*)1;
}

// Kalles for hver linje under [Vortex][Visibility], f.eks. "video=1"
static void VortexReadLine(ImGuiContext*, ImGuiSettingsHandler* Handler, void*, const char* Line)
{
    auto* Visible = (std::map<std::string, bool>*)Handler->UserData;
    const char* Equals = strchr(Line, '=');                 // Finn '='
    if (Equals == nullptr)
        return;
    std::string Name(Line, Equals - Line);                   // Teksten før '='
    (*Visible)[Name] = atoi(Equals + 1) != 0;                // Tallet etter '=': 1 = true, 0 = false
}

// Kalles når ImGui lagrer. Skriver hele dictionary-en til fila.
static void VortexWriteAll(ImGuiContext*, ImGuiSettingsHandler* Handler, ImGuiTextBuffer* Out)
{
    auto* Visible = (std::map<std::string, bool>*)Handler->UserData;
    Out->appendf("[Vortex][Visibility]\n");
    for (auto& Pair : *Visible)
        Out->appendf("%s=%d\n", Pair.first.c_str(), Pair.second ? 1 : 0);
    Out->append("\n");
}
//===============================================================================================================


// Kalles én gang etter ImGui::CreateContext(). Registrerer [Vortex]-seksjonen og laster standard-layouten.
void LayoutInit(layout_state* L, std::map<std::string, bool>* Visible)
{
    L->Visible   = Visible;
    L->ExeFolder = SDL_GetBasePath();

    ImGuiSettingsHandler VortexHandler;
    VortexHandler.TypeName   = "Vortex";                 // Navnet i [Vortex][...]
    VortexHandler.TypeHash   = ImHashStr("Vortex");
    VortexHandler.ReadOpenFn = VortexReadOpen;
    VortexHandler.ReadLineFn = VortexReadLine;
    VortexHandler.WriteAllFn = VortexWriteAll;
    VortexHandler.UserData   = Visible;                  // Gir funksjonene tilgang til dictionary-en
    ImGui::AddSettingsHandler(&VortexHandler);           // ImGui tar en kopi

    char Path[512];
    snprintf(Path, sizeof(Path), "%s../layouts/standar.ini", L->ExeFolder);
    ImGui::LoadIniSettingsFromDisk(Path);                // Standard-layout lastes ved oppstart
}

// Kalles før ImGui::NewFrame(). Layout kan ikke lastes midt i en frame.
void LayoutLoadPending(layout_state* L)
{
    if (!L->PendingLoad.empty()) {
        ImGui::LoadIniSettingsFromDisk(L->PendingLoad.c_str());
        L->PendingLoad.clear();                          // Ferdig, ikke last igjen
    }
}

// Kalles inni BeginMainMenuBar(). Tegner "Layouts"-menyen helt til høyre.
void LayoutMenu(layout_state* L)
{
    float ButtonWidth = ImGui::CalcTextSize("Layouts").x + ImGui::GetStyle().FramePadding.x * 2;
    float RightEdge   = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x;
    ImGui::SetCursorPosX(RightEdge - ButtonWidth - 10);
    if (ImGui::BeginMenu("Layouts")) {
        char Folder[512];
        snprintf(Folder, sizeof(Folder), "%s../layouts", L->ExeFolder);
        int Count = 0;
        char** Files = SDL_GlobDirectory(Folder, "*.ini", 0, &Count);
        if (Files != nullptr) {
            for (int i = 0; i < Count; i++) {
                if (ImGui::MenuItem(Files[i])) L->PendingLoad = std::string(Folder) + "/" + Files[i];  // Kopi av hele stien
            }
            SDL_free(Files);
        }

        ImGui::Separator();
        if (ImGui::BeginMenu("Lagre layout som")) {
            // Enter i feltet = lagre. Tomt navn ignoreres
            if (ImGui::InputText("Navn:", L->NewName, IM_ARRAYSIZE(L->NewName), ImGuiInputTextFlags_EnterReturnsTrue) && L->NewName[0] != '\0')
                L->PendingSave = true;                   // Lagres etter Render
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
}

// Kalles etter ImGui::Render(). Lagrer layouten hvis Enter er trykket i "Lagre layout som".
void LayoutSavePending(layout_state* L)
{
    if (L->PendingSave) {
        char Path[512];
        snprintf(Path, sizeof(Path), "%s../layouts/%s.ini", L->ExeFolder, L->NewName);
        ImGui::SaveIniSettingsToDisk(Path);
        L->PendingSave = false;
        L->NewName[0] = '\0';                            // Tøm feltet til neste gang
    }
}
