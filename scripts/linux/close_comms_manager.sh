#!/bin/bash

# Close all running comms_manager.exe processes

# Find and kill all comms_manager processes
if pgrep -f comms_manager\.exe > /dev/null; then
    echo "Found comms_manager.exe processes. Terminating..."
    pkill -f comms_manager\.exe
    sleep 1
    
    # Verify all processes are closed
    if pgrep -f comms_manager\.exe > /dev/null; then
        echo "Some processes still running. Force killing..."
        pkill -9 -f comms_manager\.exe
    fi
    
    echo "✓ All comms_manager.exe processes have been closed."
else
    echo "No comms_manager.exe processes found."
fi
