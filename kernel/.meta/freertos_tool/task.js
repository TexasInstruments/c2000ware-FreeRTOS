"use strict";
/*global exports, system*/
let CMDCommon = system.getScript("/kernel/freertos_tool/FREERTOSCommon.js");


var config = [
    {
        name: "$name",
        hidden: false,
    },
    {
        name: "taskPointer",
        displayName: "Task Function",
        default: "<Enter Task Function Name>"

    },
    {
        name: "taskStackSize",
        displayName: "Task Stack Size(words)",
        default: 128,
    },
    {
        name: "taskParams",
        displayName: "Task Parameters",
        default: "NULL",
    },
    {
        name: "taskPriority",
        displayName: "Task Priority",
        default: 0,
    },
    {
        name: "taskHandle",
        displayName: "Task Handle",
        default: "",
        hidden: false,
        getValue: (inst)=>{
            return(inst.$name + "Handle")
        }
    },
    {
        name: "taskStackBuffer",
        displayName: "Task Stack Buffer",
        default: "",
        hidden: false,
        getValue: (inst)=>{
            return(inst.$name + "StackBuffer")
        }
    },
    {
        name: "taskControlBlock",
        displayName: "Task Control Block Buffer",
        default: "",
        hidden: false,
        getValue: (inst)=>{
            return(inst.$name + "TCBBuffer")
        }
    },
    {
        name: "createDynamicTask",
        displayName: "Create Dynamic Task",
        default: false,
        onChange: (inst, ui)=>{
            if(inst.createDynamicTask == true)
            {
                ui.taskControlBlock.hidden = true;
                ui.taskStackBuffer.hidden = true;
            }
            else
            {
                ui.taskControlBlock.hidden = false;
                ui.taskStackBuffer.hidden = false;
            }
        }
    },
];



// Define the common/portable base Task
exports = {
    displayName         : "Task",
    defaultInstanceName : "myTask",
    config              : config
};
