# Arrival manager for EuroScope 
A simple arrival manager plugin for EuroScope. Uses the position predictions provided by EuroScope to visualize the arrival flow for a given airport or waypoint.

## Download

The plugin .dll-file can be found under [Releases](https://github.com/EvenAR/euroscope-aman/releases). The AMAN-display will appear in a separate window once the plugin has been loaded.

## Plugin configuration

Timelines are loaded from `aman-config.json` which must be placed in the same directory as the plugin `dll`. The file content can be reloaded at run time through the menu. 

Example `aman-config.json`:

```json
{
    "openAutomatically": false,
    "timelines": {
        "19R/19L": {
            "targetFixes": [ "GSW40", "GME40" ],
            "viaFixes": [ "ADOPI", "LUNIP", "ESEBA", "INREX", "RIPAM", "BELGU" ],
            "tagLayout": "myLayout",
            "destinationAirports": [ "ENGM" ]
        },
        "...": {
            "targetFixes": [ "....", "...." ],
            "viaFixes": [ "..." ],
            "tagLayout": "simpleLayout",
            "defaultTimeSpan": 60,
            "destinationAirports": [ "....", "...." ]
        }
    },
    "tagLayouts": {
        "myLayout": [
            { "source": "assignedRunway", "width": 4 },
            { "source": "callsign", "width": 8 },
            { "source": "static", "defaultValue": "¤", "width": 2, "isViaFixIndicator": true },
            { "source": "aircraftType", "width": 5 },
            { "source": "aircraftWtc", "width": 2 },
            { "source": "timeBehindPreceeding", "width": 5, "rightAligned": true },
            { "source": "remainingDistance", "width": 4, "rightAligned": true },
            { "source": "static", "width": 1 },
            { "source": "directRouting", "width": 5, "rightAligned": true, "defaultValue": "-----" },
            { "source": "static", "width": 1 },
            { "source": "scratchPad", "width": 4 }
        ],
        "simpleLayout": [
            { "source": "assignedRunway", "width": 4 },
            { "source": "callsign", "width": 8 }
        ]
    }
}
```

Note: the plugin uses the EuroScope font which is based on the [ANSI/Windows-1252 character encoding](http://www.alanwood.net/demos/ansi.html). If you want to use special symbols like ¶, ¤, •, |, ©, ®, ¬, ‡ or º in your "static" tag fields you must save the JSON-file using that encoding (select "ANSI" encoding in notepad). Also note that some of these symbols have a custom representation in the EuroScope font - like ¶ which is displayed as a telephone, and ¤ which is a filled rectangle.

### Timelines

| Property         | Description
|------------------|---------------
| `targetFixes`    | Based on the assigned route, any aircraft expected to pass one of these fixes are shown in the timeline. When exactly two fixes are specified, a dual timeline is shown with the first fix on the left side and the second on the right side.
| `tagLayout`      | The id of the tag-layout that should be used for this timeline.
| `viaFixes`       | (optional) Each fix will be assigned a color, and aircraft with a route initially (any direct routings ignored) going through one of these fixes will be marked with the color. For example, this can give a better overview of which direction each aircraft is coming from. Only eight different colors are available at the moment.
| `defaultTimeSpan`| (optional) If used, this will be the initial "zoom"-level (in minutes) when the timeline is loaded.
| `destinationAirports` | (optional) If used, aircraft whose destination is not in `destinationAirports` will not be included.

### Tag layouts

A tag layout has a set of tag values, which will be drawn in the specified order. Each tag value is configured using the following properties:

| Property            | Description
|---------------------|---------------
| `source`            | Where to get the value from (see table below).
| `width`             | The number of characters that the value should "reserve". If the value is longer than `width` it will be truncated.
| `rightAligned`      | (optional) Defaults to `false`.
| `isViaFixIndicator` | (optional) If true, the value will be colored based on the "via fix". Defaults to `false`.
| `defaultValue`      | (optional) Can only be used if `source` is `directRouting` or `static`. Defaults to `""`.

The following `source`s are available:

| Name                             | Description
|----------------------------------|---------------
| `callsign`                       | Aircraft call sign.
| `assignedRunway`                 | Assigned landing runway.
| `assignedStar`                   | Assigned STAR.
| `aircraftType`                   | ICAO aircraft code.
| `aircraftWtc`                    | Wake turbulence category (L/M/H/S).
| `minutesBehindPreceedingRounded` | Time behind preceeding aircraft (rounded to nearest minute).
| `timeBehindPreceeding`           | Time behind preceeding aircraft (mm:ss).
| `remainingDistance`              | Distance to target fix (nautical miles).
| `estimatedLandingTime`           | Estimated landing time (hh:mm).
| `directRouting`                  | Direct routing (if any) given by ATC.
| `groundSpeed`                    | Calculated ground speed.
| `groundSpeed10`                  | Calculated ground speed (in tens).
| `altitude`                       | Altitude (pressure altitude or FL).
| `scratchPad`                     | Scratch pad value.
| `static`                         | A static text, specified in the `defaultValue` property.

## Available dot-commands

| Command             | Description
|---------------------|---------------
| `.aman open`        | Opens the window 
| `.aman close`       | Closes the window

## Building the project

Microsoft Visual Studio Community can be used to build and debug the plugin. Note that it must be compiled as x86 (32 bit) which is the architecture EuroScope is based on.

## Screenshot

![Window](https://i.gyazo.com/abd832a844331f03635ee72e5562ee13.png)
