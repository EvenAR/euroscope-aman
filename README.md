# Arrival manager for EuroScope 
A simple arrival manager plugin for EuroScope. Uses the position predictions provided by EuroScope to visualize the arrival flow for a given airport or waypoint.

## Download

The plugin .dll-file can be found in the Release folder. [Direct link](https://github.com/EvenAR/euroscope-aman/raw/master/Release/Aman.dll).
The AMAN-display will appear in a separate window once the plugin has been loaded. 

## Configuration

Timelines are loaded from `aman-config.json` which must be placed in the same directory as the plugin `dll`. The file content can be reloaded at run time through the menu. 

Example `aman-config.json`:

```json
{
    "tagLayouts": {
        "myLayout": [
            { "source": "assignedRunway", "minWidth": 4 },
            { "source": "callsign", "minWidth": 8 },
            { "source": "custom", "defaultValue": "*", "minWidth": 2, "isViaFixIndicator": true },
            { "source": "aircraftType", "minWidth": 5 },
            { "source": "aircraftWtc", "minWidth": 2 },
            { "source": "timeBehindPreceeding", "minWidth": 5, "align": "right" },
            { "source": "remainingDistance", "minWidth": 4, "align": "right" },
            { "source": "directRouting", "minWidth": 6, "align": "right", "defaultValue": "-----" }
        ]
    },
    "timelines": {
        "19R/19L": {
            "targetFixes": [ "GSW40", "GME40" ],
            "viaFixes": [ "ADOPI", "LUNIP", "ESEBA", "INREX", "RIPAM", "BELGU" ],
            "tagLayout": "myLayout"
        }
    }
}
```

### Timelines

| Property         | Description
|------------------|---------------
| `targetFixes`    | Based on the assigned route, any aircraft expected to pass one of these fixes are shown in the timeline. When exactly two fixes are specified, a dual timeline is shown with the first fix on the left side and the second on the right side.
| `tagLayout`      | The id of the tag-layout that should be used for this timeline.
| `viaFixes`       | (optional) Each fix will be assigned a color, and aircraft with a route initially (any direct routings ignored) going through one of these fixes will be marked with the color. For example, this can give a better overview of which direction each aircraft is coming from. Only eight different colors are available at the moment.
| `startHorizon`   | (optional) If used, this will be the initial time horizon (in minutes) when the timeline is loaded.
| `destinationAirports` | (optional) If used, aircraft whose destination is not in `destinationAirports` will not be included.

### Tag layouts

A tag layout has a set of tag values, which will be drawn in the listed order. Each tag value is configured using the following properties:

| Property            | Description
|---------------------|---------------
| `source`            | Where to get the value from. The following sources are available: `callsign`, `assignedRunway`, `aircraftType`, `aircraftWtc`, `minutesBehindPreceedingRounded`, `timeBehindPreceeding`, `remainingDistance`, `directRouting`, `custom`.
| `minWidth`          | The number of characters that the value should "reserve". The number should be at least the longest output you expect (eg. for `aircraftType` use `4` or greater).
| `rightAligned`      | (optional) Defaults to `false`.
| `isViaFixIndicator` | (optional) If true, the value will be colored based on the "via fix". Defaults to `false`.
| `defaultValue`      | (optional) Can only be used if `source` is `directRouting` or `custom`. Defaults to `""`.


![Window](https://i.gyazo.com/52cf2fbc1d6eb48f4a77b71784e7c61f.png)

Search tags:
> Vatsim, air traffic control, ATC, flight simulator
