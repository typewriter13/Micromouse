# Test Codes

Incremental development stages, kept for reference. Each folder is a standalone ESP-IDF project representing one step on the way to the final firmware in [`LeftWallFollow`](../LeftWallFollow/).

| Folder | What it tests |
| :--- | :--- |
| [`motor`](motor) | Both motors driven straight at a fixed duty, no sensors |
| [`IRtest`](IRtest) | All 4 IR channels read and printed, no motor output |
| [`irmotorcontrol`](irmotorcontrol) | Stop one wheel steering off raw front IR readings |
| [`pid`](pid) | First PD wall-centering + diagonal-based turning, no ToF |
| [`tof-pid`](tof-pid) | PD centering combined with ToF front-wall stopping |
