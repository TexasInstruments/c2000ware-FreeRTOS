let references = system.getScript("/kernel/freertos_tool/FREERTOSReferences.js");
var freertos_export = {}

if ([ "F2837xD",
      "F2837xS",
      "F2807x",
      "F28004x",
      "F28003x",
      "F2838x",
      "F28002x",
      "F280013x",
      "F280015x",
      "F28P65x"].includes(system.deviceData.device))
{
    freertos_export = {
        displayName: "FreeRTOS Configuration Tool",
        templates:
        [
            {
                name      : "/kernel/freertos_tool/templates/c2000_freertos.c.xdt",
                outputPath: "c2000_freertos.c",
            },
            {
                name      : "/kernel/freertos_tool/templates/c2000_freertos.h.xdt",
                outputPath: "c2000_freertos.h",
            },
            {
                name      : "/kernel/freertos_tool/templates/c2000_freertos_source.c.xdt",
                outputPath: "c2000_freertos_source.c",
            },
            {
                name      : "/kernel/freertos_tool/templates/c2000_freertos_asm.asm.xdt",
                outputPath: "c2000_freertos_asm.asm",
            },
            {
                name      : "/kernel/freertos_tool/templates/FreeRTOSConfig.h.xdt",
                outputPath: "FreeRTOSConfig.h",
            },
            {
                name      : "/kernel/freertos_tool/templates/c2000_freertos.cmd.genlibs.xdt",
                outputPath: "c2000_freertos.cmd.genlibs",
            },
            {
                name      : "/kernel/freertos_tool/templates/c2000_freertos.opt.xdt",
                outputPath: "c2000_freertos.opt",
            },
        ],
        references: references.componentReferences,
        topModules:
        [
            {
                displayName: "FreeRTOS Configuration",
                description: "Configure FreeRTOS.",
                modules:
                [
                    "/kernel/freertos_tool/FREERTOS",
                ]
            }
        ]
    }
}

exports = freertos_export