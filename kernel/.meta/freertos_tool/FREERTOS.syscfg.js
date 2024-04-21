"use strict";
/*global exports, system*/
let Common   = system.getScript("/driverlib/Common.js");
let Pinmux   = system.getScript("/driverlib/pinmux.js");
let CMDCommon = system.getScript("/kernel/freertos_tool/FREERTOSCommon.js");

/* Intro splash on GUI */
let longDescription = `FreeRTOS Configuration Tool`;

function onChangeEnableROV(inst, ui)
{
    if (inst.enableROV == true)
    {
        inst.SUPPORT_DYNAMIC_ALLOCATION = true;
        inst.MAX_TASK_NAME_LEN = 16;
        inst.QUEUE_REGISTRY_SIZE = 10;
        inst.USE_QUEUE_SETS = false;
        inst.USE_TRACE_FACILITY = true;
        inst.CHECK_FOR_STACK_OVERFLOW = 2;   
        // Heap type needs to be 4, add check
    }
    else
    {
        inst.USE_TRACE_FACILITY = false;
        inst.CHECK_FOR_STACK_OVERFLOW = 0;
    }
}

var config = [
    // Folder select
    {
        name: "useExternalInstall",
        displayName: "Use External FreeRTOS Install",
        default: false,
        onChange: (inst, ui)=>{
            ui.freertosPath.hidden = !inst.useExternalInstall
        }
    },
    {
        name: "freertosPath",
        displayName: "FreeRTOS Install Path",
        default: "",
        fileFilter: ".*",
        hidden: true,
        pickDirectory: true,
    },
    {
        name: "enableROV",
        displayName: "Enable ROV settings",
        description: "ROV/RTOS Objects is a debugging tool part of CCS",
        onChange: onChangeEnableROV,
        default: false,
    },
    {
        name: "GROUP_FREERTOS_CONFIG",
        displayName: "FreeRTOS Configuration",
        collapsed: true,
        config: [
        {
            name: "START_SCHEDULER" ,
            displayName: "Start scheduler after initialization",
            description: "Start FreeRTOS scheduler from within the Sysconfig initialization code",
            default: true,
        },    
        {
        name: "GROUP_KERNEL_CONFIG",
        displayName: "Kernel Configuration",
        collapsed: false,
        config: [
            {
                name: "CPU_CLOCK_HZ" ,
                displayName: "CPU Clock Hz",
                description: "",
                default: 100000000,
            },
            {
                name: "TICK_RATE_HZ" ,
                displayName: "Tick Rate Hz",
                description: "",
                default: 1000,
            },
            {
                name: "MAX_PRIORITIES" ,
                displayName: "Max Priorities",
                description: "",
                default: 16,
            },
            {
                name: "MINIMAL_STACK_SIZE" ,
                displayName: "Minimal Stack Size",
                description: "",
                default: 128,
            },
            {
                name: "MAX_TASK_NAME_LEN" ,
                displayName: "Max Task Name Length",
                description: "",
                default: 16,
            },
            {
                name: "QUEUE_REGISTRY_SIZE" ,
                displayName: "Queue Registry Size",
                description: "",
                default: 10,
            },
            {
                name: "TASK_NOTIFICATION_ARRAY_ENTRIES" ,
                displayName: "Task Notification Array Entries",
                description: "",
                default: 3,
            },
            {
                name: "USE_PREEMPTION" ,
                displayName: "Use Preemption",
                description: "",
                default: true,
            },
            {
                name: "USE_TIME_SLICING" ,
                displayName: "Use Time Slicing",
                description: "",
                default: true,
            },
            {
                name: "USE_PORT_OPTIMISED_TASK_SELECTION" ,
                displayName: "Use Port Optimised Task Selection",
                description: "",
                default: false,
            },
            {
                name: "USE_TICKLESS_IDLE" ,
                displayName: "Use Tickless Idle",
                description: "",
                default: false,
            },
            {
                name: "EXPECTED_IDLE_TIME_BEFORE_SLEEP" ,
                displayName: "Expected idle time before sleep",
                description: "",
                default: 2,
            },
            {
                name: "USE_16_BIT_TICKS" ,          
                displayName: "Use 16 Bit Ticks",
                description: "",
                default: false,
            },
            /* {
                name: "TICK_TYPE_WIDTH_IN_BITS" ,           // To be updated
                displayName: "Tick type width in bits",
                description: "",
                default     : 'TICK_TYPE_WIDTH_32_BITS',
                options     : [
                    {name: "TICK_TYPE_WIDTH_16_BITS"},
                    {name: "TICK_TYPE_WIDTH_32_BITS"},
                    {name: "TICK_TYPE_WIDTH_64_BITS"},
                ]
            }, */
            {
                name: "IDLE_SHOULD_YIELD" ,
                displayName: "Idle Should Yield",
                description: "",
                default: false,
            },
            {
                name: "USE_TASK_NOTIFICATIONS" ,
                displayName: "Use Task Notifications",
                description: "",
                default: true,
            },
            {
                name: "USE_MUTEXES" ,
                displayName: "Use Mutexes",
                description: "",
                default: false,
            },
            {
                name: "USE_RECURSIVE_MUTEXES" ,
                displayName: "Use Recursive Mutexes",
                description: "",
                default: false,
            },
            {
                name: "USE_COUNTING_SEMAPHORES" ,
                displayName: "Use Counting Semaphores",
                description: "",
                default: false,
            },
            {
                name: "USE_QUEUE_SETS" ,
                displayName: "Use Queue Sets",
                description: "",
                default: false,
            },
            {
                name: "ENABLE_BACKWARD_COMPATIBILITY" ,
                displayName: "Enable Backward Compatibility",
                description: "",
                default: false,
            },
            {
                name: "NUM_THREAD_LOCAL_STORAGE_POINTERS" ,
                displayName: "Number of indexes in each task's TLS array",
                description: "",
                default: 0,
            },
            {
                name: "USE_MINI_LIST_ITEM" ,
                displayName: "Use mini list items in lists",
                description: "",
                default: true,
            },
            {
                name: "RECORD_STACK_HIGH_ADDRESS" ,
                displayName: "Record Stack High Address",
                description: "",
                default: false,
            },
        ]
    },
    {
        name: "GROUP_MEMORY_CONFIG",
        displayName: "Memory Configuration",
        collapsed: false,
        config: [
            {
                name: "SUPPORT_STATIC_ALLOCATION" ,
                displayName: "Support Static Allocation",
                description: "",
                default: true,
            },
            {
                name: "SUPPORT_DYNAMIC_ALLOCATION" ,
                displayName: "Support Dynamic Allocation",
                description: "",
                default: true,
                onChange: (inst, ui)=>{
                    if(inst.SUPPORT_DYNAMIC_ALLOCATION == true)
                    {
                        ui.HEAP_TYPE.hidden = false;
                    }
                    else
                    {
                        ui.HEAP_TYPE.hidden = true;
                    }
                }
            },
            {
                name: "HEAP_TYPE" ,
                displayName: "Heap Type",
                description: "",
                default: "heap_4",
                options: [
                    {name: "heap_1", displayName: "Heap 1"},
                    {name: "heap_2", displayName: "Heap 2"},
                    {name: "heap_3", displayName: "Heap 3"},
                    {name: "heap_4", displayName: "Heap 4"},
                    {name: "heap_5", displayName: "Heap 5"}
                ]
            },
            {
                name: "APPLICATION_ALLOCATED_HEAP" ,
                displayName: "Application Allocated Heap",
                description: "",
                default: true,
            },
            {
                name: "STACK_ALLOCATION_FROM_SEPARATE_HEAP" ,
                displayName: "Stack Allocation From Separate Heap",
                description: "",
                default: false,
            },
            {
                name: "TOTAL_HEAP_SIZE" ,
                displayName: "Total Heap Size",
                description: "",
                default: 1024,
            },

        ]
    },
    {
        name: "GROUP_HOOK_FXN_CONFIG",
        displayName: "Hook/Callback Configuration",
        collapsed: false,
        config: [
            {
                name: "USE_IDLE_HOOK" ,
                displayName: "Use Idle Hook",
                description: "",
                default: false,
            },
            {
                name: "USE_TICK_HOOK" ,
                displayName: "Use Tick Hook",
                description: "",
                default: false,
            },
            {
                name: "CHECK_FOR_STACK_OVERFLOW" ,
                displayName: "Check For Stack Overflow",
                description: "",
                default: 0,
                options: [
                    {name: 0, displayName: "0 (Disabled)"},
                    {name: 1, displayName: "1"},
                    {name: 2, displayName: "2"},
                ]
            },
            {
                name: "USE_MALLOC_FAILED_HOOK" ,
                displayName: "Use Malloc Failed Hook",
                description: "",
                default: false,
            },
            {
                name: "USE_DAEMON_TASK_STARTUP_HOOK" ,
                displayName: "Use Daemon Task Startup Hook",
                description: "",
                default: false,
            },
        ]
    },
    {
        name: "GROUP_STATS_CONFIG",
        displayName: "Statistics Configuration",
        collapsed: false,
        config: [
            {
                name: "GENERATE_RUN_TIME_STATS" ,
                displayName: "Generate Run Time Stats",
                description: "",
                default: false,
            },
            {
                name: "USE_TRACE_FACILITY" ,
                displayName: "Use Trace Facility",
                description: "",
                default: false,
            },
            {
                name: "USE_STATS_FORMATTING_FUNCTIONS" ,
                displayName: "Use Stats Formatting Functions",
                description: "",
                default: false,
            },
        ]
    },
    {
        name: "GROUP_COROUTINE_CONFIG",
        displayName: "Co-routine Configuration",
        collapsed: false,
        config: [
            {
                name: "USE_CO_ROUTINES" ,
                displayName: "Use Co-Routines",
                description: "",
                default: false,
            },
                        {
                name: "MAX_CO_ROUTINE_PRIORITIES" ,
                displayName: "Max Co-Routine Priorities",
                description: "",
                default: 0,
            },

        ]
    },
    {
        name: "GROUP_SWTIMER_CONFIG",
        displayName: "Software Timer Configuration",
        collapsed: false,
        config: [
            {
                name: "USE_TIMERS",
                displayName: "Use Timers",
                description: "",
                default: false,
            },
            {
                name: "TIMER_TASK_PRIORITY",
                displayName: "Timer Task Priority",
                description: "",
                default: 3
            },
            {
                name: "TIMER_QUEUE_LENGTH",
                displayName: "Timer Queue Length",
                description: "",
                default: 10
            },
            {
                name: "TIMER_TASK_STACK_DEPTH",
                displayName: "Timer Task Stack Depth",
                description: "",
                default: 0
            },


        ]
    },
    {
        name: "GROUP_RTOS_INCLUDE",
        displayName: "FreeRTOS Optional API Include",
        description: "Include optional APIs",
        collapsed: false,
        config: [
            {
                name: "vTaskPrioritySet",
                displayName: "vTaskPrioritySet",
                description: "",
                default: false
            },
            {
                name: "uxTaskPriorityGet",
                displayName: "uxTaskPriorityGet",
                description: "",
                default: false
            },
            {
                name: "vTaskDelete",
                displayName: "vTaskDelete",
                description: "",
                default: false
            },
            {
                name: "vTaskSuspend",
                displayName: "vTaskSuspend",
                description: "",
                default: true
            },
            {
                name: "xResumeFromISR",
                displayName: "xResumeFromISR",
                description: "",
                default: false
            },
            {
                name: "vTaskDelayUntil",
                displayName: "vTaskDelayUntil",
                description: "",
                default: true
            },
            {
                name: "vTaskDelay",
                displayName: "vTaskDelay",
                description: "",
                default: true
            },
            {
                name: "xTaskGetSchedulerState",
                displayName: "xTaskGetSchedulerState",
                description: "",
                default: false
            },
            {
                name: "xTaskGetCurrentTaskHandle",
                displayName: "xTaskGetCurrentTaskHandle",
                description: "",
                default: false
            },
            {
                name: "uxTaskGetStackHighWaterMark",
                displayName: "uxTaskGetStackHighWaterMark",
                description: "",
                default: false
            },
            {
                name: "uxTaskGetStackHighWaterMark2",
                displayName: "uxTaskGetStackHighWaterMark2",
                description: "",
                default: false
            },
            {
                name: "xTaskGetIdleTaskHandle",
                displayName: "xTaskGetIdleTaskHandle",
                description: "",
                default: false
            },
            {
                name: "eTaskGetState",
                displayName: "eTaskGetState",
                description: "",
                default: false
            },
            {
                name: "xEventGroupSetBitFromISR",
                displayName: "xEventGroupSetBitFromISR",
                description: "",
                default: false
            },
            {
                name: "xTimerPendFunctionCall",
                displayName: "xTimerPendFunctionCall",
                description: "",
                default: false
            },
            {
                name: "xTaskAbortDelay",
                displayName: "xTaskAbortDelay",
                description: "",
                default: false
            },
            {
                name: "xTaskGetHandle",
                displayName: "xTaskGetHandle",
                description: "",
                default: false
            },
            {
                name: "xTaskResumeFromISR",
                displayName: "xTaskResumeFromISR",
                description: "",
                default: false
            },
            /* {
                name: "vTaskCleanUpResources",          // Obsolete API
                displayName: "vTaskCleanUpResources",
                description: "",
                default: false
            }, */
        ]
    }
 ]
},
];


var moduleInstances = (inst) => {
    var mods = []
    mods.push({
        name: "tasks",
        displayName: "User Task Configurations",
        moduleName: "/kernel/freertos_tool/task.js",
        requiredArgs: {
            maxNameLength           : inst.MAX_TASK_NAME_LEN,
            maxPriority             : inst.MAX_PRIORITIES,
        },
        useArray: true,
        collapsed: true,
    })
    mods.push({
        name: "semaphores",
        displayName: "Semaphore/Mutex Configurations",
        moduleName: "/kernel/freertos_tool/semaphore.js",
        useArray: true,
        collapsed: true,
        requiredArgs: {
            isMutexEnabled          : inst.USE_MUTEXES,
            isRecMutexEnabled       : inst.USE_RECURSIVE_MUTEXES,
            isCountingSemEnabled    : inst.USE_COUNTING_SEMAPHORES
        },
    })
    mods.push({
        name: "queues",
        displayName: "Queue Configurations",
        moduleName: "/kernel/freertos_tool/queue.js",
        useArray: true,
        collapsed: true,
    })

    mods.push({
        name: "timers",
        displayName: "Timer Configurations",
        moduleName: "/kernel/freertos_tool/timer.js",
        useArray: true,
        collapsed: true,
        requiredArgs: {
            isTimerEnabled      : inst.USE_TIMERS
        },
    })
    mods.push({
        name: "events",
        displayName: "Event Configurations",
        moduleName: "/kernel/freertos_tool/event.js",
        useArray: true,
        collapsed: true,
    })

    mods.push({
        name: "tickTimer",
        displayName: "Tick Timer",
        description: "The CPU Timer used for FreeRTOS tick (Configured as part of xPortStartScheduler function)",
        moduleName: "/driverlib/cputimer.js",
        requiredArgs: {
            $name              : "timer2",
            cputimerBase       : CPUTIMER_INSTANCE[2].name,
            enableInterrupt    : true,
            registerInterrupts : true,
            startTimer         : false,
            timerPeriod        : 100000,
            timerPrescaler     : 0,
            clockPrescaler     : "CPUTIMER_CLOCK_PRESCALER_1",
            clockSource        : "CPUTIMER_CLOCK_SOURCE_SYS",
            timerInt           : {
                interruptHandler: "portTICK_ISR"
            },
        },
    })
    return mods
}

var CPUTIMER_INSTANCE = [
    { name: "CPUTIMER0_BASE", displayName: "CPUTIMER0"},
    { name: "CPUTIMER1_BASE", displayName: "CPUTIMER1"},
    { name: "CPUTIMER2_BASE", displayName: "CPUTIMER2"},
]

function sharedModuleInstances (inst)
{
    var sharedModules = []
    // sharedModules = sharedModules.concat([{
    //     name: "tickTimer",
    //     displayName: "Tick Timer",
    //     description: "The CPU Timer used for FreeRTOS tick",
    //     moduleName: "/driverlib/cputimer.js",
    //     requiredArgs: {
    //         $name              : "timer2",
    //         cputimerBase       : CPUTIMER_INSTANCE[2].name,
    //         enableInterrupt    : true,
    //         registerInterrupts : true,
    //         startTimer         : false,
    //     }
    // }])

    return sharedModules;
}

function onValidate(inst, validation)
{
    if(inst.USE_PORT_OPTIMISED_TASK_SELECTION == true)
    {
        validation.logError(
            "Port optimized task selection is not supported!",inst,"USE_PORT_OPTIMISED_TASK_SELECTION");
    }

    if(inst.APPLICATION_ALLOCATED_HEAP == false)
    {
        validation.logError(
            "Application allocated heap should be enabled to avoid memory issues!", inst, "APPLICATION_ALLOCATED_HEAP");
    }
    
    if(inst.IDLE_SHOULD_YIELD == true && inst.USE_PREEMPTION == false)
    {
        validation.logWarning(
            "Preemptive scheduler must be enabled to take effect",inst,"IDLE_SHOULD_YIELD");
    }

    if(inst.USE_DAEMON_TASK_STARTUP_HOOK == true && inst.USE_TIMERS == false)
    {
        validation.logWarning(
            "Timer usage must be enabled to take effect",inst,"USE_DAEMON_TASK_STARTUP_HOOK");
    }

    if(inst.SUPPORT_STATIC_ALLOCATION == true)
    {
        validation.logInfo(
            "Update minimal stack size to update Idle task stack size!", inst, "MINIMAL_STACK_SIZE");
    }

    if((inst.USE_TIMERS == true) && (inst.TIMER_TASK_STACK_DEPTH == 0))
    {
        validation.logError(
            "Update timer task stack depth!", inst, "TIMER_TASK_STACK_DEPTH");
    }

    if (inst.enableROV == true){
        if(inst.SUPPORT_DYNAMIC_ALLOCATION == false)
            validation.logWarning(
                "Must be on for full ROV functionality",inst,"SUPPORT_DYNAMIC_ALLOCATION");
        if(inst.MAX_TASK_NAME_LEN != 16)
            validation.logWarning(
                "Must be 16 for full ROV functionality",inst,"MAX_TASK_NAME_LEN");
        if(inst.QUEUE_REGISTRY_SIZE < 1)
            validation.logWarning(
                "Must be >=1 for full ROV functionality",inst,"QUEUE_REGISTRY_SIZE");
        if(inst.USE_QUEUE_SETS == true)
            validation.logWarning(
                "Must be false for full ROV functionality",inst,"USE_QUEUE_SETS");
        if(inst.USE_TRACE_FACILITY == false)
            validation.logWarning(
                "Must be on for full ROV functionality",inst,"USE_TRACE_FACILITY");
        if(inst.CHECK_FOR_STACK_OVERFLOW < 2)
            validation.logWarning(
                "Must be 2 for full ROV functionality",inst,"CHECK_FOR_STACK_OVERFLOW");
    }
}

// Define the common/portable base FreeRTOS
var freeRTOSModule = {
    displayName         : "FREERTOS",
    description         : "FREERTOS",
    defaultInstanceName : "myFREERTOS",
    longDescription     : longDescription,
    maxInstances        : 1,
    moduleInstances     : moduleInstances,
    sharedModuleInstances: sharedModuleInstances,
    modules: (inst) => {
        var mods = []
        if (inst) {
            if (inst.useExternalInstall)
            {
                mods.push({
                    name: "pullInTemplateFREERTOS_SOURCE_C",
                    moduleName: "/kernel/freertos_tool/templates/c2000_freertos_source.c.dynamic.js",
                })
                mods.push({
                    name: "pullInTemplateFREERTOS_ASM",
                    moduleName: "/kernel/freertos_tool/templates/c2000_freertos_asm.asm.dynamic.js",
                })
            }
            else
            {
                mods.push({
                    name: "pullInTemplateFREERTOS_SOURCE",
                    moduleName: "/kernel/freertos_tool/templates/c2000_freertos_references.dynamic.js",
                })
                if(inst.SUPPORT_DYNAMIC_ALLOCATION == true){
                    mods.push({
                        name: "pullInHeap",
                        moduleName: "/kernel/freertos_tool/templates/heap/"+String(inst.HEAP_TYPE)+".js",
                    })
                }
            }
            return mods;
        }
        return [];
    },
    templates: {
        "/kernel/freertos_tool/templates/c2000_freertos.c.xdt" : "",
        "/kernel/freertos_tool/templates/c2000_freertos.h.xdt" : "",
        "/kernel/freertos_tool/templates/FreeRTOSConfig.h.xdt" : "",
        "/kernel/freertos_tool/templates/c2000_freertos.cmd.genlibs.xdt" : "",
        "/kernel/freertos_tool/templates/c2000_freertos.opt.xdt": "",
        "/kernel/freertos_tool/templates/syscfg_c.rov.xs.xdt": ""
    },
    config              : config,
    validate            : onValidate,
};

exports = freeRTOSModule;
