#include "cglm/cglm.h"
#include "Log.h"
#include "Platform.h"

bool bRunning = true;

double gSimTime = 0.0;

void Render(double alpha)
{

}

void Update(double dt)
{

}

int main(int argc, const char** argv)
{
    struct LogArgs args =
    {
        .bIncludeLogTimeStamps = true,
        .bLogTextColoured = true,
        .bLogToConsole = true,
        .logfilePath = NULL
    };
    Log_Init(&args);

    int platformSuccess = Platform_Init();
    if(platformSuccess != 0)
    {
        Log_Error("Platform couldn't initialize - shutting down");
        return 1;
    }

    const double dt = 1.0 / 60.0; // fixed simulation step
    double accumulator = 0.0;
    double current_time = Platform_GetTime();

    while (!Platform_ShouldWindowClose())
    {
        double new_time = Platform_GetTime();
        double frame_time = new_time - current_time;
        current_time = new_time;

        // clamp to avoid spiral of death on a huge hitch (e.g. breakpoint, alt-tab)
        if (frame_time > 0.25) frame_time = 0.25;

        accumulator += frame_time;

        while (accumulator >= dt) {
            Update(dt);       // deterministic, fixed-size steps
            accumulator -= dt;
            gSimTime += dt;
        }

        double alpha = accumulator / dt; // fraction into current step, 0..1
        Render(alpha);

        Platform_SwapBuffers();

        Platform_PollInput(); // where you put this matters — see below
    }
    
}
