"use strict";
/*global exports, system*/
let CMDCommon = system.getScript("/kernel/freertos_tool/FREERTOSCommon.js");


function onValidate(inst, validation)
{
    if(inst.isTimerEnabled == false)  
    {
        validation.logError(
            "Timer must be enabled in FreeRTOSConfig.h",inst);
    }
}

var config = [
    {
        name: "$name",
        hidden: false,
    },
    {
        name: "isTimerEnabled",
        default: false,
        hidden: true,
    },
    {
        name: "timerPeriod",
        displayName: "Timer Period(ticks)",
        default: "pdMS_TO_TICKS(10)"
    },
    {
        name: "timerAutoReload",
        displayName: "Timer Auto Reload",
        default: true
    },
    {
        name: "timerId",
        displayName: "Timer ID",
        default: "(void *) 0"
    },
    {
        name: "timerCallbackFunction",
        displayName: "Timer Callback Function",
        default: "<Enter Callback Function Name>"
    },
    {
        name: "timerHandle",
        displayName: "Timer Handle",
        default: "NULL",
        getValue: (inst)=>{
            return(inst.$name + "Handle")
        }
    },
    {
        name: "timerBuffer",
        displayName: "Timer Buffer",
        default: "NULL",
        getValue: (inst)=>{
            return(inst.$name + "Buffer")
        }
    },
    {
        name: "createDynamicTimer",
        displayName: "Create Dynamic Timer",
        default: false,
        onChange: (inst, ui)=>{
            if(inst.createDynamicTimer == true)
            {
                ui.timerBuffer.hidden = true;
            }
            else
            {
                ui.timerBuffer.hidden = false;
            }
        }

    }
];



// Define the common/portable base Timer
exports = {
    displayName         : "Timer",
    defaultInstanceName : "myTimer",
    config              : config,
    validate            : onValidate,
};
