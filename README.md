# Arrival manager for EuroScope 
A simple arrival manager plugin for EuroScope. Uses the position predictions provided by EuroScope to visualize the arrival flow for a given airport or waypoint.

## How to use
The AMAN-display will appear in a separate window once the plugin has been loaded. Timelines are loaded from `aman-config.json` which must be placed in the same directory as the plugin `dll`. The file content can be reloaded at run time through the menu. 

Example `aman-config.json`:

```json
{
    "timelines": [
        {
            "alias": "19R/19L",
            "targetFixes": [ "GSW40", "GME40" ],
            "viaFixes": [ "ADOPI", "LUNIP", "ESEBA", "INREX", "BELGU", "RIPAM" ],
            "initialHorizon": 120
        },
        {
            "alias": "...",
            "targetFixes": [ "....", "...." ],
            "viaFixes": [ "..." ],
            "initialHorizon": ...
        }
    ]
}
```

| Property         | Description
|------------------|---------------
| `targetFixes`    | Based on the assigned route, any aircraft expected to pass one of these fixes are shown in the timeline. When exactly two fixes are specified, a dual timeline is shown with the first fix on the left side and the second on the right side.
| `viaFixes`       | (optional) Each fix will be assigned a color, and aircraft with a route initially (any direct routings ignored) going through one of these fixes will be marked with the color. For example, this can give a better overview of which direction each aircraft is coming from. Only eight different colors are available at the moment.
| `alias`          | (optional) If used, this will be the ID of the timeline. If not, the name will be generated from `targetFixes`.
| `initialHorizon` | (optional) If used, this will be the initial time horizon (in minutes) when the timeline is loaded.

The information displayed for each aircraft has the following layout:

![](https://i.gyazo.com/76f58bf5317288c11fdf2580356c913b.png)

```
<assigned runway>
<call sign>
<"via fix" indicator> (when applicable)
<aircraft type>
<wake turbulence category>
<assigned direct fix>
<minutes behind preceeding aircraft>
<distance to final target fix>
```

## Download

The plugin .dll-file can be found in the Release folder. [Direct link](https://github.com/EvenAR/euroscope-aman/raw/master/Release/Aman.dll).

![Window](https://i.gyazo.com/52cf2fbc1d6eb48f4a77b71784e7c61f.png)
