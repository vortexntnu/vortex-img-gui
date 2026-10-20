#include <vector>
#include <math.h>

void drawControlPlot(bool* open) {
    static float xs1[101],ys1[101];

    for(int i =0;i<101;++i){
        xs1[i]=i*0.001f;
        ys1[i]=0.5f+0.5f*sinf(50*(xs1[i]+(float)ImGui::GetTime()/10));
    }

    if (ImGui::Begin("Control plot", open)) {
        if (ImPlot::BeginPlot("Control sin fine graf", ImVec2(-1, -1))) {
            ImPlot::SetupAxes("x", "y");
            ImPlot::PlotLine("f(x)", xs1, ys1, 101);
            ImPlot::EndPlot();
        }
    }
    ImGui::End();   // alltid, også når Begin returnerer false
}