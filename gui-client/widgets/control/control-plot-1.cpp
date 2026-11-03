#include <vector>
#include <math.h>

void drawControlPlot_1(bool* open) {
    static float xs1[101],ys1[101];

    for(int i =0;i<101;++i){
        xs1[i]=i*0.001f;
        ys1[i]=ImGui::GetTime();
    }

    if (ImGui::Begin("Control plot-1", open)) {
        if (ImPlot::BeginPlot("Control plot-1", ImVec2(-1, -1))) {
            ImPlot::SetupAxes("x", "y", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            ImPlot::PlotLine("f(x)", xs1, ys1, 101);
            ImPlot::EndPlot();
        }
    }
    ImGui::End();   // alltid, også når Begin returnerer false
}