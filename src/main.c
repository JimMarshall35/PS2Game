#include "cglm/cglm.h"
#include "Log.h"

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
    
}
