#include "includes.h"

static const char PROGRAM_NAME[]    = "building-process-demo";
static const char PROGRAM_VERSION[] = "1.0.0";

int main(int argc, char *argv[])
{
    print_str(PROGRAM_NAME);
    print_str(" v");
    print_str(PROGRAM_VERSION);
    print_chr('\n');
    
    return 0;
}