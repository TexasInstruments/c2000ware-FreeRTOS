"use strict";
/*global exports, system*/
let CMDCommon = system.getScript("/kernel/freertos_tool/FREERTOSCommon.js");


var config = [
    {
        name: "$name",
        hidden: false,
    },
    {
        name: "queueLength",
        displayName: "Queue Length",
        default: 1,
    },
    {
        name: "queueItemSize",
        displayName: "Queue Item Size (bytes)",
        default: "sizeof(uint16_t)",
    },
    {
        name: "queueHandle",
        displayName: "Queue Handle",
        default: "NULL",
        getValue: (inst)=>{
            return(inst.$name + "Handle")
        }
    },
    {
        name: "queueStorageBuffer",
        displayName: "Queue Storage Buffer",
        default: "NULL",
        getValue: (inst)=>{
            return(inst.$name + "StorageBuffer")
        }
    },
    {
        name: "queueBuffer",
        displayName: "Queue Buffer",
        default: "NULL",
        getValue: (inst)=>{
            return(inst.$name + "Buffer")
        }
    },
    {
        name: "createDynamicQueue",
        displayName: "Create Dynamic Queue",
        default: false,
        onChange: (inst, ui)=>{
            if(inst.createDynamicQueue == true)
            {
                ui.queueStorageBuffer.hidden = true;
                ui.queueBuffer.hidden = true;
            }
            else
            {
                ui.queueStorageBuffer.hidden = false;
                ui.queueBuffer.hidden = false;
            }
        }
    }
];



// Define the common/portable base Watchdog
exports = {
    displayName         : "Queue",
    defaultInstanceName : "myQueue",
    config              : config
};
