# Arrival manager for EuroScope 
A simple arrival manager plugin for EuroScope. Uses the position predictions provided by EuroScope to visualize the arrival flow for a given airport or waypoint.

## Commands
* `.aman show` opens window if it's been closed.

### Add a timeline
* `.aman add WAYPT1` adds a timeline for a single waypoint.
* `.aman add WAYPT1/WAYPT2` adds a timeline shared by two waypoints, where aircraft inbound for `WAYPT1` are shown on the left side and inbounds to `WAYPT2` are shown on the right side.
* `.aman add WAYPT1 30` adds a timeline showing a 30 minute large window (default is 60 minutes).

### Remove a timeline
* `.aman del WAYPT1` or `.aman del WAYPT1/WAYPT2`
 
![alt text](https://i.gyazo.com/307103c8a24ecec491e2709dfdf86087.png)
