# Arrival manager for EuroScope 
A simple arrival manager plugin for EuroScope. Uses the position predictions provided by EuroScope to visualize the arrival flow for a given airport or waypoint.

## Usage
The AMAN-display will appear in a separate window once the plugin has been loaded.
* `.aman show` opens window if it's been closed.

### Add a timeline
* `.aman add WAYPT1` adds a timeline for a single waypoint.
* `.aman add WAYPT1/WAYPT2` adds a split view with, where aircraft inbound for `WAYPT1` are shown on the left side and inbounds to `WAYPT2` are shown on the right side.
* `.aman add WAYPT1/WAYPT2/WAYPT3/...` adds a timeline showing inbounds for multiple waypoints.
* `.aman add <waypoints> VIA1,VIA2,VIA3,...` adds a timeline with call signs colored based on which via-waypoint the aircraft is coming from. 

### Remove a timeline
* `.aman del X` where X is the position of the timeline you want to remove, counting from the left (1 = the leftmost). 
* `.aman clear` removes all timelines.

## Download
- Note: This is an early version of the plugin. It is recommended to load the plugin in a secondary instance of EuroScope (connected to VATSIM via proxy) in case the plugin causes EuroScope to crash.

The plugin .dll-file can be found in the Release folder. [Direct link](https://github.com/EvenAR/euroscope-aman/raw/master/Release/Aman.dll).


![alt text](https://i.gyazo.com/84338383130d1a59cedba452c61fc1a6.png)
