"use strict";
/*global exports, system*/
let CMDCommon = system.getScript("/kernel/freertos_tool/FREERTOSCommon.js");

var config = [
    {
        name: "$name",
        hidden: false,
    },
    {
        name: "eventHandle",
        displayName: "Event Group Handle",
        default: "NULL",
        getValue: (inst)=>{
            return(inst.$name + "Handle")
        }
    },
    {
        name: "eventGroupBuffer",
        displayName: "Event Group Buffer",
        default: "NULL",
        hidden: false,
        getValue: (inst)=>{
            return(inst.$name + "Buffer")
        }
    },
    {
        name: "createDynamicEvent",
        displayName: "Create Dynamic Event Group",
        default: false,
        onChange: (inst, ui)=>{
            if(inst.createDynamicEvent == true)
            {
                ui.eventGroupBuffer.hidden = true;
            }
            else
            {
                ui.eventGroupBuffer.hidden = false;
            }
        }
    }
];



// Define the common/portable base Event
exports = {
    displayName         : "Event",
    defaultInstanceName : "myEvent",
    config              : config
};
