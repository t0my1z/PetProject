
#include "CommsManager.hpp"

int main(int argc, char** argv) 
{
    CommsManager commsManager;
    commsManager.Initialize(argc, argv);
    commsManager.Loop();
    commsManager.Shutdown();

    return 0;
}