"use strict";
/*global exports, system*/
let CMDCommon = system.getScript("/kernel/freertos_tool/FREERTOSCommon.js");


var config = [
    {
        name: "$name",
        hidden: false,
    },
    {
        name: "semType",
        displayName: "Semaphore Type",
        hidden      : false,
        default     : "BINARY_SEMAPHORE",
        options     : [
            {name: "MUTEX_SEMAPHORE", displayName: "Mutex"},
            {name: "RECURSIVE_MUTEX_SEMAPHORE", displayName: "Recursive Mutex"},
            {name: "BINARY_SEMAPHORE", displayName: "Binary Semaphore"},
            {name: "COUNTING_SEMAPHORE", displayName: "Counting Semaphore"},
        ],
        onChange: (inst, ui)=>{
            if(inst.semType == "COUNTING_SEMAPHORE")
            {
                ui.semMaxCount.hidden = false
                ui.semInitialCount.hidden = false
            }
        }
    },
    {
        name: "semMaxCount",
        displayName: "Maximum Count value",
        hidden: true,
        default: 0,
    },
    {
        name: "semInitialCount",
        displayName: "Initial Count value",
        hidden: true,
        default: 0,
    },
    {
        name: "semHandle",
        displayName: "Semaphore Handle",
        default: "NULL",
        getValue: (inst)=>{
            return(inst.$name + "Handle")
        }
    },
    {
        name: "semBuffer",
        displayName: "Semaphore Buffer",
        default: "NULL",
        getValue: (inst)=>{
            return(inst.$name + "Buffer")
        }
    },
    {
        name: "createDynamicSem",
        displayName: "Create Dynamic Semaphore",
        default: false,
        onChange: (inst, ui)=>{
            if(inst.createDynamicSem == true)
            {
                ui.semBuffer.hidden = true;
            }
            else
            {
                ui.semBuffer.hidden = false;
            }
        }
    }
];



// Define the common/portable base Semaphore
exports = {
    displayName         : "Semaphore",
    defaultInstanceName : "mySemaphore",
    config              : config
};
