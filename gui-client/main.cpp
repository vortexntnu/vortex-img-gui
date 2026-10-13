#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include<imgui.cpp>
#include<imgui_demo.cpp>
#include<imgui_draw.cpp>
#include<imgui_tables.cpp>
#include<imgui_widgets.cpp>
#include <imgui_impl_sdl3.cpp>
#include <imgui_impl_opengl3.cpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <stdio.h>
#include"../lib/typedef.h"
#include"../lib/drone_state.h"
#include"../lib/platform.h"

#include "video.cpp"

#include <map>      // std::map (dictionary)
#include <string>   // std::string

bool isButtonPressed(ImGuiKey key){
    bool pressed = ImGui::IsKeyDown(key);
    return pressed;
}

#include <vector>

std::vector<std::string> guiConsoleLines; //lager liste (vektor) som tar vare på "string tall" til listen guiConsoleLines
int consoleNumberLine = 0; //deklarerer

void toGUIConsole(std::string text){ 
    consoleNumberLine+=1;
    guiConsoleLines.push_back(std::to_string(consoleNumberLine) + ": " + text);
}


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



int main(int, char**)
{
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }

    // Select GL version + let the backend select a GLSL version
    const char* glsl_version = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);9
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + generally GLSL 150
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + generally GLSL 130
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow("Vortex GUI client", (int)(1920/2 * main_scale), (int)(1080/2 * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }

    //=================================================== VORTEX LOGO IKON ( taskbar + programmvindu ) ==========
    SDL_Surface* icon = SDL_LoadPNG("asset/icon.png");
    if (icon)
    {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }
    else
    {
        printf("Kunne ikke laste ikon: %s\n", SDL_GetError());
    }
    //===============================================================================================================

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.IniFilename = nullptr;

    // Hvilke vinduer som er åpne. Lagres og lastes sammen med layout-filene
    std::map<std::string, bool> Visible;

    ImGuiSettingsHandler VortexHandler;
    VortexHandler.TypeName   = "Vortex";                 // Navnet i [Vortex][...]
    VortexHandler.TypeHash   = ImHashStr("Vortex");
    VortexHandler.ReadOpenFn = VortexReadOpen;
    VortexHandler.ReadLineFn = VortexReadLine;
    VortexHandler.WriteAllFn = VortexWriteAll;
    VortexHandler.UserData   = &Visible;                 // Gir funksjonene tilgang til dictionary-en
    ImGui::AddSettingsHandler(&VortexHandler);

    const char* ExeFolder = SDL_GetBasePath(); // f.eks. "...\gui-client\BuildOutput\"
    char Path[512];
    snprintf(Path, sizeof(Path), "%s../layouts/standar.ini", ExeFolder); 
    ImGui::LoadIniSettingsFromDisk(Path);    // Layout 1 lastes ved oppstart 
    std::string PendingLoadLayout;                   // Full sti til layout som skal lastes. Tom = ingenting
    s32 PendingSaveLayout = 0;                       // 0 = ingenting skal lagres

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.92549f, 0.1098f, 0.1294f, 1.0f); // rød når aktiv
    style.Colors[ImGuiCol_TitleBg]       = ImVec4(0.36235f, 0.1098f, 0.1294f, 1.0f); // mørkere rød når ikke aktiv


    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- Video: ringbuffer, tekstur og kamera-tråd ---
    const s32 VideoW = 640, VideoH = 480;
    frame_buffer VideoFrames;
    FrameBufferInit(&VideoFrames, VideoW, VideoH);

    GLuint VideoTex;
    glGenTextures(1, &VideoTex);                      // Én tekstur som gjenbrukes for alle frames
    glBindTexture(GL_TEXTURE_2D, VideoTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VideoW, VideoH, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);  // Bare reserverer plass, ingen data ennå

    std::atomic<bool> CameraRunning(true);
    std::atomic<bool> NextImage(false);               // Settes når W trykkes, leses av kamera-tråden
    std::thread CameraThread(TestCameraThread, &VideoFrames, &CameraRunning, &NextImage);

   
    // Our state

    ImVec4 clear_color = ImVec4(0.051f, 0.082f, 0.216f, 1.00f); // Mørk blå bakgrunnsfarge

    int ellevillVariabel = 0;
    bool isWPressed= false;




    static float valueFloat = 0.5;
    static int valueInt = 10;
    static char buffer[64] = "";
    char ny[64] = "";
    

    


    PlatformInitializeNetworkLib();
    platform_ip ServerIP = PlatformIP("127.0.0.1");
    platform_socket Server = PlatformConnect(ServerIP, 2100);
    u64 TimeSiceNetworkConnect = SDL_GetTicks();
    // Main loop
    bool done = false;
    bool tilkoblet = false;
    bool guiConsoleWindow = false;

    


#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif 
    {
        if (!Server.IsValid && ImGui::IsKeyPressed(ImGuiKey_C, true)) {
            u64 ThisTime = SDL_GetTicks();
            if (ThisTime - TimeSiceNetworkConnect > 1000) {
                Server = PlatformConnect(ServerIP, 2100);
                TimeSiceNetworkConnect = ThisTime;
            }
        }

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        
        
        if (!PendingLoadLayout.empty()) {
            ImGui::LoadIniSettingsFromDisk(PendingLoadLayout.c_str());
            PendingLoadLayout.clear();               // Ferdig, ikke last igjen
        }
        

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // W bytter til neste bilde i videovinduet (ikke mens man skriver i et tekstfelt)
        if (ImGui::IsKeyPressed(ImGuiKey_W, false) && !io.WantTextInput){
            NextImage = true;
            toGUIConsole("W pressed, switching picture.");
        }    
        {
            drone_state DroneState = {};
            PlatformRecv(&Server,(u8*)&DroneState,sizeof(DroneState));

            static float f = 0.0f;
            static int counter = 0;
        }
        {   
            
             

            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.3f, 0.05f, 0.05f, 1.0f));
            ImGui::Begin("Vortex NTNU official W key press detection");
            ImGui::PopStyleColor(2);

            isWPressed = isButtonPressed(ImGuiKey_W);
            
            ImGui::Text("er w ned? = %d",isWPressed);

            ImGui::SetCursorPosX(60);

            
            ImGui::InputText("W text?",buffer,sizeof(buffer));
            ImGui::Text(buffer);

            ImGui::End();

            if (ImGui::BeginMainMenuBar()){
                ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x);
                if (Server.IsValid){                 
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "On");
                }
                else{
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Off");
                }

                if(ImGui::BeginMenu("Autonomy")){
                    if(ImGui::MenuItem("Video")) Visible["video"] = !Visible["video"];

                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("Control")){
                    if(ImGui::MenuItem("Video")) Visible["video"] = !Visible["video"];
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("Perception")){
                    if(ImGui::MenuItem("Video")) Visible["video"] = !Visible["video"];
                    ImGui::EndMenu();
                }
                if(ImGui::BeginMenu("GUI-Console")){
                    if(ImGui::MenuItem("Console")) Visible["console"] = !Visible["console"];
                    //guiConsoleWindow = !guiConsoleWindow;
                    
                    ImGui::EndMenu();
                }
                

                float ButtonWidth = ImGui::CalcTextSize("Layouts").x + ImGui::GetStyle().FramePadding.x * 2;
                float RightEdge   = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x;
                ImGui::SetCursorPosX(RightEdge - ButtonWidth - 10);
                if (ImGui::BeginMenu("Layouts")) {
                    char Folder[512];
                    snprintf(Folder, sizeof(Folder), "%s../layouts", ExeFolder);
                    int Count = 0;
                    char** Files = SDL_GlobDirectory(Folder, "*.ini", 0, &Count);
                    if (Files != nullptr) {
                        for (int i = 0; i < Count; i++) {
                            if (ImGui::MenuItem(Files[i])) PendingLoadLayout = std::string(Folder) + "/" + Files[i];  // Kopi av hele stien
                        }
                        SDL_free(Files);
                        
                }

                    //Oppdatere
                    ImGui::Separator();
                    if (ImGui::BeginMenu("Lagre layout som")) {
                        // Enter i feltet = lagre. Tomt navn ignoreres
                        if (ImGui::InputText("Navn:", ny, IM_ARRAYSIZE(ny), ImGuiInputTextFlags_EnterReturnsTrue) && ny[0] != '\0')
                            PendingSaveLayout = 1;   // Lagres etter Render
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();             
                }

                ImGui::EndMainMenuBar();          
            }

            if (Visible["video"]){

                ImGui::Begin("Memes", &Visible["video"]);   // X i vinduet setter Visible["video"] = false
                u8* FramePixels;
                if (GetLatestFrame(&VideoFrames, &FramePixels))   // Bare når det finnes en ny frame
                {
                    glBindTexture(GL_TEXTURE_2D, VideoTex);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VideoW, VideoH,
                                    GL_RGBA, GL_UNSIGNED_BYTE, FramePixels); // Oppdaterer samme tekstur
                }
                float VideoDisplayW = ImGui::GetContentRegionAvail().x;           // Full bredde i vinduet
                float VideoDisplayH = VideoDisplayW * VideoH / VideoW;            // Høyde som beholder 4:3
                ImGui::Image((ImTextureID)(intptr_t)VideoTex, ImVec2(VideoDisplayW, VideoDisplayH));
                ImGui::End();

            }

            //Console window
            if (Visible["console"]){ 
                ImGui::Begin("GUI-Console", &guiConsoleWindow);
                for (const std::string& line : guiConsoleLines){
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", line.c_str());
                }
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f); // auto-scroll nedover
                ImGui::End();
            }

            
            ImGui::Begin("Generell");

            ImGui::End();
            


        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        // Lagre layout når Enter er trykket i "Lagre layout som"
        if (PendingSaveLayout) {
            char Path[512];
            snprintf(Path, sizeof(Path), "%s../layouts/%s.ini", ExeFolder, ny);
            ImGui::SaveIniSettingsToDisk(Path);
            PendingSaveLayout = 0;
            ny[0] = '\0';                            // Tøm feltet til neste gang
        }
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup

    CameraRunning = false;          // Ber kamera-tråden stoppe
    CameraThread.join();            // Venter til den er ferdig, før bufferne frigjøres
    glDeleteTextures(1, &VideoTex);
    FrameBufferFree(&VideoFrames);

    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
