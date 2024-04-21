"use strict";
/*global exports, system*/
let CMDCommon = system.getScript("/kernel/freertos_tool/FREERTOSCommon.js");


function onChangeAddTaskParams(inst, ui)
{
    if (inst.addTaskParams == "Constant parameter")
    {
        ui.taskParams.readOnly = false;
        inst.taskParams = "<Enter Task Parameter Value>";
    }
    else if (inst.addTaskParams == "Variable parameter")
    {
        ui.taskParams.readOnly = false;
        inst.taskParams = "<Enter Task Parameter Name>";
    }
    else 
    {
        inst.taskParams = "NULL";
        ui.taskParams.readOnly = true;
    }
}


function onValidate(inst, validation)
{
    if(inst.$name.length > inst.maxNameLength - 1)  
    {
        validation.logWarning(
            "Name is longer than max task name length",inst,"$name");
    }

    if(inst.taskPriority > inst.maxPriority - 1)  
    {
        validation.logWarning(
            "Exceeding max allowed priority",inst,"taskPriority");
    }
}

var config = [
    {
        name: "$name",
        hidden: false,
    },
    {
        name: "maxNameLength",
        default: 0,
        hidden: true,
    },
    {
        name: "maxPriority",
        default: 0,
        hidden: true,
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
        name: "addTaskParams",
        displayName: "Specify Task Parameters",
        default: "Constant parameter" ,
        options     : [
            {name: "None (NULL parameter)"},
            {name: "Constant parameter"},
            {name: "Variable parameter"},
        ],
        onChange: onChangeAddTaskParams
    },
    {
        name: "taskParams",
        displayName: "Task Parameters",
        default: "NULL",
        readOnly: false,
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
    config              : config,
    validate            : onValidate
};
