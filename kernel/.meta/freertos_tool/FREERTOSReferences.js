var references = [
        {
            name: "TASK",
            path: "../kernel/FreeRTOS/Source/tasks.c",
            alwaysInclude: false,
        },
        {
            name: "QUEUE",
            path: "../kernel/FreeRTOS/Source/queue.c",
            alwaysInclude: false,
        },
        {
            name: "LIST",
            path: "../kernel/FreeRTOS/Source/list.c",
            alwaysInclude: false,
        },
        {
            name: "TIMER",
            path: "../kernel/FreeRTOS/Source/timers.c",
            alwaysInclude: false,
        },
        {
            name: "EVENTS",
            path: "../kernel/FreeRTOS/Source/event_groups.c",
            alwaysInclude: false,
        },
        {
            name: "SBUFFER",
            path: "../kernel/FreeRTOS/Source/stream_buffer.c",
            alwaysInclude: false,
        },
        {
            name: "PORT",
            path: "../kernel/FreeRTOS/Source/portable/CCS/C2000_c28x/port.c",
            alwaysInclude: false,
        },
        {
            name: "PORTASM",
            path: "../kernel/FreeRTOS/Source/portable/CCS/C2000_c28x/portasm.asm",
            alwaysInclude: false,
        },
        {
            name: "HEAP4",
            path: "../kernel/FreeRTOS/Source/portable/MemMang/heap_4.c",
            alwaysInclude: false,
        },
        //
        // H files are not really necessary as they can be included with a -I in the opt file
        //
]


function getReferencePath(name)
{
    for (var ref of references)
    {
        if (ref.name == name)
        {
            return ref.path
        }
    }
}

var componentReferences = []
for (var ref of references)
{
    componentReferences.push({
        path: ref.path,
        alwaysInclude: ref.alwaysInclude
    })
}

exports = {
    references: references,
    getReferencePath : getReferencePath,
    componentReferences : componentReferences
}