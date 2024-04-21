"use strict";
/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


Object.defineProperty(exports, "__esModule", { value: true });
class Heap {
}
class TaskModule {
}
class TaskInstance {
}
class QueueInstance {
}
class SemaphoreInstance {
}
class MutexInstance {
}
class TimerInstance {
}
class FreeRTOS {
    constructor(ctx) {
        this.ctx = ctx;
        this.viewMap = [
            { name: 'Heap Overview', fxn: this.getHeap.bind(this), structName: Heap },
            { name: 'Task Overview', fxn: this.getTaskModule.bind(this), structName: TaskModule },
            { name: 'Tasks', fxn: this.getTaskInstances.bind(this), structName: TaskInstance },
            { name: 'Queues', fxn: this.getQueueInstances.bind(this), structName: QueueInstance },
            { name: 'Semaphores', fxn: this.getSemaphoreInstances.bind(this), structName: SemaphoreInstance },
            { name: 'Mutexes', fxn: this.getMutexInstances.bind(this), structName: MutexInstance },
            { name: 'Timers', fxn: this.getTimers.bind(this), structName: TimerInstance },
        ];
        this.Program = this.ctx.getProgram();
    }
    
    async getHeap() {
        const view = [];
        const heapInfo = new Heap();
        try {
            let current = await this.Program.fetchVariable('xStart');
            const end = await this.Program.fetchVariable('pxEnd');
            const ucHeapType = await this.Program.lookupTypeByVariable('ucHeap');
            // Update with equivalent function
            // heapInfo.HeapAddress = (await this.Program.lookupSymbolValueDynamic('ucHeap'));
            heapInfo.HeapType = 'heap_4';
            if (end == 0) {
                heapInfo.TotalSize = 'Heap not initialized';
            }
            else {
                heapInfo.TotalSize = ucHeapType.elnum; // sizeof(BlockLink_t)
                let minEverFreeRem = await this.Program.fetchVariable("xMinimumEverFreeBytesRemaining");
                heapInfo.PeakUsage = heapInfo.TotalSize - minEverFreeRem;
                heapInfo.FirstFreeBlock = current.pxNextFreeBlock;
                while (current.pxNextFreeBlock < end) {
                    current = await this.Program.fetchFromAddr(current.pxNextFreeBlock, 'BlockLink_t');
                    heapInfo.TotalFree = (heapInfo.TotalFree ?? 0) + current.xBlockSize;
                }
            }
            // theia - make FirstFreeBlock to hex
            if (typeof heapInfo.FirstFreeBlock === 'number') {
                heapInfo.FirstFreeBlock = '0x' + heapInfo.FirstFreeBlock.toString(16);
            }
        }
        catch (e) {
            heapInfo.HeapType = 'Heap implementation not supported in ROV (currently only heap_4 is)';
        }
        view.push(heapInfo);
        return (view);
    }
    async getTaskModule() {
        const view = [];
        const taskInfo = new TaskModule();
        taskInfo.TickCount = await this.Program.fetchVariable("xTickCount");
        const readyList = await this.Program.fetchVariable('pxReadyTasksLists');
        taskInfo.NumPriorities = readyList.length;
        taskInfo.NumTasks = await this.Program.fetchVariable('uxCurrentNumberOfTasks');
        taskInfo.TopReadyPriority    = await this.Program.fetchVariable("uxTopReadyPriority");
        taskInfo.NumOverflows        = await this.Program.fetchVariable("xNumOfOverflows");
        taskInfo.SchedulerStarted    = Boolean(await this.Program.fetchVariable("xSchedulerRunning"));
        taskInfo.State               = await this.Program.fetchVariable("uxSchedulerSuspended") ? "Suspended" : "Running";
        
        view.push(taskInfo);
        return (view);
    }
    async getTaskInstances() {
        const table = [];
        const readyList = await this.Program.fetchVariable('pxReadyTasksLists');

        for (let i = 0; i < readyList.length; i++) {
            await this.fillInTaskInstance(table, readyList[i], 'Ready');
        }
        
        const delay1List = await this.Program.fetchVariable('xDelayedTaskList1');
        await this.fillInTaskInstance(table, delay1List, 'Blocked');
        const delay2List = await this.Program.fetchVariable('xDelayedTaskList2');
        await this.fillInTaskInstance(table, delay2List, 'Blocked');
        
        try {
            const suspendedList = await this.Program.fetchVariable('xSuspendedTaskList');
            await this.fillInTaskInstance(table, suspendedList, 'Suspended');
        }
        catch (e) {

        }
        try {
            const terminatedList = await this.Program.fetchVariable('xTasksWaitingTermination');
            await this.fillInTaskInstance(table, terminatedList, 'Terminated');
        }
        catch (e) {

        }

        table.sort(compareAddress);
        return (table);
    }
    async fillInTaskInstance(table, list, state) {
        const currentTask = await this.Program.fetchVariable('pxCurrentTCB');
        let tcbBase = list.xListEnd.pxNext - 2;
        let queueMapList = null;
        for (let i = 0; i < list.uxNumberOfItems; i++) {
            const task = await this.Program.fetchFromAddr(tcbBase, 'TCB_t');
            const taskInfo = new TaskInstance();
            // Commenting out since currently the task function is stored on the
            // stack and generally is wiped out. Maybe when we get CallStack we
            // can use it to get the entry function.
            // var nameAddr = tcbBase - 24;
            // var functionAddr = Program.fetchFromAddr(nameAddr, "uintptr_t");
            taskInfo.Address = '0x' + tcbBase.toString(16);
            let name = '';
            for (let j = 0; j < 16; j++) {
                if (task.pcTaskName[j] == 0)
                    break;
                name = name + String.fromCharCode(task.pcTaskName[j]);
            }
            taskInfo.TaskName = name;
            // taskInfo.FxnName = String(Program.lookupFuncName(functionAddr));
            taskInfo.Priority = task.uxPriority;
            taskInfo.BasePriority = task.uxBasePriority;
            if (tcbBase == currentTask) {
                taskInfo.State = 'Running';
            }
            else {
                taskInfo.State = state;
            }

            if (state == "Blocked") {
                /* We only need to create the queueMapList once each time
                 * the task module is rendered (and some task is blocked)
                 * thus initializing it once here */
                if (queueMapList == null) {
                    queueMapList = await this.createMapForEachQueueObject();
                }

                if (queueMapList == null) {
                    taskInfo.BlockedOn = "Unknown";
                }
                else{
                    try{
                        let flag=0;
                        for (let i = 0; i < queueMapList.length; i++) {
                            if (checkIfTaskIsBlockingOnQueue(tcbBase, queueMapList[i])) {
                                taskInfo.BlockedOn = queueMapList[i]["Type"] + ": " + queueMapList[i]["Name"];
                                flag++;
                                break;
                            }
                        }
                        if(!flag){
                            taskInfo.BlockedOn = "Unknown";
                        }
                    }
                    catch(e){
                        taskInfo.BlockedOn = "Unknown";
                    }
                }
            }
            else{
                taskInfo.BlockedOn = "-";
            }

            taskInfo.MutexesHeld = task.uxMutexesHeld;
            taskInfo.StackBase = task.pxStack;
            taskInfo.CurrentTaskSP = task.pxTopOfStack;

            // We don't know the size of the task stack, so look every 4 bytes :(
            // The stack size is stored in the "malloc" header right before the
            // stack (for dynamically allocated stacks). Tmr Svc and IDLE tasks
            // stack size probably can be queried from the target. These can be
            // used to improve the performance (e.g. read the whole stack instead
            // a word at a time!).
            let stackData = await this.Program.fetchArray('8u', task.pxStack, 2);
            let index = task.pxStack;
            // Find the first 0x00a500a5.
            while ( (stackData[0] != 0x00a5) &&
                    (stackData[1] != 0x00a5) &&
                    (index <= task.pxEndOfStack)) {
                index += 2;
                stackData = await this.Program.fetchArray('8u', index, 2);
            }

            //!! C28x port uses portSTACK_GROWTH > 0

            if ((index >= task.pxEndOfStack) ||
                (taskInfo.CurrentTaskSP >= task.pxEndOfStack)) {
                taskInfo.UnusedStackSize = 'Stack Overflow';
            }
            else {
                //index -= 2;
                //taskInfo.UnusedStackSize = index - taskInfo.StackBase;
                taskInfo.UnusedStackSize = task.pxEndOfStack - index;
            }

            // theia - make StackBase, CurrentTaskSP to hex
            if (typeof taskInfo.StackBase === 'number') {
                taskInfo.StackBase = '0x' + taskInfo.StackBase.toString(16);
            }
            if (typeof taskInfo.CurrentTaskSP === 'number') {
                taskInfo.CurrentTaskSP = '0x' + taskInfo.CurrentTaskSP.toString(16);
            }

            try{
                let staticallyAlloc = task.ucStaticallyAllocated;
                if (staticallyAlloc == 2) {
                    taskInfo.StaticAlloc = "Stack & TCB";
                }
                else if (staticallyAlloc == 1) {
                    taskInfo.StaticAlloc = "Stack, not TCB";
                }
                else if (staticallyAlloc == 0) {
                    taskInfo.StaticAlloc = "No";
                }
            }
            catch(e){
                /* ucStaticallyAllocated was not present, likely because of that
                 * tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE is not set */
                taskInfo.StaticAlloc = "No";
            }

            table.push(taskInfo);
            // Traverse the list
            tcbBase = task.xStateListItem.pxNext - 2;
        }
    }
    async getTimers() {
        const table = [];
        let errVal = 0;

        try{
            const activeList1 = await this.Program.fetchVariable("xActiveTimerList1");
            await this.fillInTimerInstance(table, activeList1);
        }
        catch(e){
            errVal++;
        }
        try{
            const activeList2 = await this.Program.fetchVariable("xActiveTimerList2");
            await this.fillInTimerInstance(table, activeList2);
        }
        catch(e){   
            errVal++;
        }

        if(errVal == 2){
            let message = new TimerInstance();
            message.Address = "No active timers, or configUSE_TIMERS is set to 0";
            table.push(message);
            return (table);
        }

        table.sort(compareHandle);
        return table;
    }

    async fillInTimerInstance(table, list) {
        if(list.uxNumberOfItems > 0)
        {
            let listItem = await this.Program.fetchFromAddr(list.xListEnd.pxNext, "ListItem_t");
            for (let i = 0; i < list.uxNumberOfItems; i++)
            {
                let timer = await this.Program.fetchFromAddr(listItem.pvOwner, "Timer_t");
                let timerInfo = new TimerInstance();

                timerInfo.Handle = '0x' + listItem.pvOwner.toString(16);
                timerInfo.Name = await this.helperReadStringFromAddr(timer.pcTimerName, 16);
                timerInfo.PeriodInTicks = timer.xTimerPeriodInTicks;
                timerInfo.AutoReload = "No";
                timerInfo.Active = "No";
                timerInfo.StaticallyAloc = "No";

                if( timer.ucStatus & 0x1 )
                    timerInfo.Active = "Yes";
                if( timer.ucStatus & 0x2 )
                    timerInfo.StaticallyAloc = "Yes";    
                if( timer.ucStatus & 0x4 )
                    timerInfo.AutoReload = "Yes";

                timerInfo.CallbackAddress = (timer.pxCallbackFunction != 0) ? '0x' + timer.pxCallbackFunction.toString(16) : "-" ;
                timerInfo.TimerID = timer.pvTimerID;

                table.push(timerInfo);

                if( i < (list.uxNumberOfItems-1))
                {
                    // Traverse the list
                    listItem = await this.Program.fetchFromAddr(listItem.pxNext, "ListItem_t");
                }
            }
        }
    }

    async helperReadStringFromAddr(ptr, maxLen){
        let name = "";
        let arr = await this.Program.fetchFromAddr(ptr, "char", maxLen);
        for (let i = 0; i < arr.length; i++) {
            if (arr[i] == 0) break;
            name += String.fromCharCode(arr[i]);
        }
        return name;
    }

    async helperGetListOfAddressesInListObj(listObj){
        let currentItem = await this.Program.fetchFromAddr(listObj.xListEnd.pxNext, "ListItem_t");
        let list = [];
    
        for (let i = 0; i < listObj.uxNumberOfItems; i++) {
            let address = currentItem.pvOwner;
            list.push(address);
    
            /* Traverse the list */
            currentItem = await this.Program.fetchFromAddr(currentItem.pxNext, "ListItem_t");
        }
    
        return list;
    }

    getTaskName(pcTaskName){
        let name = "";
        /* Max length of a task name is defined in FreeRTOSConfig.h
        * with the label configMAX_TASK_NAME_LEN, it is set to 16 */
        for (var j = 0; j < 16; j++) {
            if (task.pcTaskName[j] == 0) break;
            name += String.fromCharCode(task.pcTaskName[j]);
        }
        return name;
    }

    async getQueueInstances() {
        const table = [];
        let queueMaps = await this.createMapForEachQueueObject();

        /* queueMaps will be null here when configQUEUE_REGISTRY_SIZE
        * is <= 0 thus we add a message to guide the user */
        if (queueMaps == null) {
            let message = new QueueInstance();
            message.Address = "Set configQUEUE_REGISTRY_SIZE to the maximum number of queues + semaphores + mutexes that ROV should be able to read";
            table.push(message);
            return (table);
        }

        for (let i = 0; i < queueMaps.length; i++) {
            await this.fillQueueObjectFromMap(queueMaps[i], table);
        }

        table.sort(compareAddress);
        return (table);
    }

    async createMapForEachQueueObject(){
        let xQueueRegistry;

        try {
            xQueueRegistry = await this.Program.fetchVariable("xQueueRegistry");
        }
        catch(e) {
            /* xQueueRegistry couldn't be found */
            /* Most likely configQUEUE_REGISTRY_SIZE is set to 0 */
            return null;
        }
    
        let maps = [];
    
        for (let i = 0; i < xQueueRegistry.length; i++) {
            /* The queue registry holds queue registry items*/
            let QRItem = xQueueRegistry[i];
            if (QRItem.pcQueueName != 0 && QRItem.xHandle != 0) {
                /* Set limit on 16 characters for name. If the name is longer,
                 * we get the first 16 characters. This choice of max length is
                 * arbitrary and may be changed */
                let name = await this.helperReadStringFromAddr(QRItem.pcQueueName, 16);
                let map  = await this.parseQueueObjManually(name, QRItem.xHandle);
                maps.push(map);
            }
        }
    
        return maps;
    }

    async parseUnionInQueueStruct(pointerToUnion, queueType){
        let unionMap = {};
    
        /* Queue has queueType 0 anything else {1, 2, 3, 4} is a Mutex or Semaphore */
        if (queueType != 0) {
            /* This queue is used as a Mutex or Semaphore
             * Now, parse the union */
            let taskAddress = Number(await this.Program.fetchFromAddr(pointerToUnion, "TaskHandle_t", 1));
            if (taskAddress == 0) {
                // When a mutex is not held by any task the taskAddress will be zero 
                unionMap["xMutexHolder"] = null;
            }
            else {
                let task = await this.Program.fetchFromAddr(taskAddress, "TCB_t");
                let name = '';
                for (let j = 0; j < 16; j++) {
                    if (task.pcTaskName[j] == 0) break;
                    name = name + String.fromCharCode(task.pcTaskName[j]);
                }
                let addr = '0x' + taskAddress.toString(16);
                unionMap["xMutexHolder"] = name + " : " + addr;
            }
            unionMap["uxRecursiveCallCount"] = await this.Program.fetchFromAddr(pointerToUnion + 2, "UBaseType_t", 1); 
        }
        else {
            /* This queue is used as a Queue
             * Now, parse the union */
            unionMap["pcTail"]     = await this.Program.fetchFromAddr(pointerToUnion, "uint32_t", 1);
            unionMap["pcReadFrom"] = await this.Program.fetchFromAddr(pointerToUnion + 2, "uint32_t", 1);
        }
    
        return unionMap;
    }

    async parseQueueObjManually(name, xHandle){

        let currentPtr = xHandle;
    
        /* Mirror the Queue_t (QueueDefinition) struct */
    
        /* Assumes 32bit machine */
        let pcHead                 = await this.Program.fetchFromAddr(currentPtr, "uint32_t", 1);
        currentPtr                 += 2; //increment in bytes

        let pcWriteTo              = await this.Program.fetchFromAddr(currentPtr, "uint32_t", 1);
        currentPtr                 += 2; 


       //!! Defining offsets for C28x
        let SemaphoreDataSize      = 4
        let QueuePointersSize      = 4
        let BaseSize               = 1
        let ListSize               = 10
     
        /* Ignore union until we know queue type */
        let pointerAtUnion         = currentPtr;
        currentPtr                 += Math.max(SemaphoreDataSize, QueuePointersSize);
    
        let xTasksWaitingToSend    = await this.Program.fetchFromAddr(currentPtr, "List_t", 1);
        currentPtr                 += ListSize;
    
        let xTasksWaitingToReceive = await this.Program.fetchFromAddr(currentPtr, "List_t", 1);
        currentPtr                 += ListSize; 
    
        let uxMessagesWaiting      = await this.Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
        currentPtr                 += BaseSize;
    
        let uxLength               = await this.Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
        currentPtr                 += BaseSize;
        
        let a = currentPtr;
        let uxItemSize             = await this.Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
        currentPtr                 += BaseSize;

    
        //let CRxLock                = await this.Program.fetchFromAddr(currentPtr, "int8_t", 1);
        //currentPtr                 += 1;
    
        //let CTxLock                = await this.Program.fetchFromAddr(currentPtr, "int8_t", 1);
        //currentPtr                 += 1;
        currentPtr                 += 1; 

        //!! configSUPPORT_DYNAMIC_ALLOCATION needs to be 1, which is standard in TI software 
    
        let ucStaticallyAllocated  = await this.Program.fetchFromAddr(currentPtr, "uint8_t", 1);
        currentPtr                 += 1; 
        
        //!! configUSE_QUEUE_SETS     needs to be 0, which is standard in TI software 
        //!! configUSE_TRACE_FACILITY needs to be 1, which is standard in TI software

        /* eslint-disable-next-line no-unused-vars */
        /* let uxQueueNumber          = await this.Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
        currentPtr                 += BaseSize; */
    
        /* Both TICLANG and GCC puts 1 byte of padding here */
        //currentPtr += 1;
    
        currentPtr                 += 2;

        let ucQueueType  = await this.Program.fetchFromAddr(currentPtr, "uint8_t", 1);        
        let unionMap     = await this.parseUnionInQueueStruct(pointerAtUnion, ucQueueType);
    
        let map = {};
    
        map["Address"]                  = '0x' + xHandle.toString(16);
        map["Name"]                     = name;
        map["uxLength"]                 = uxLength;
        map["Type"]                     = getQueueType(ucQueueType);
        map["uxMessagesWaiting"]        = uxMessagesWaiting;
        map["TasksWaitingToReceive"]    = xTasksWaitingToReceive.uxNumberOfItems;
        map["TasksWaitingToSend"]       = xTasksWaitingToSend.uxNumberOfItems;
        map["ucStaticallyAllocated"]    = Boolean(ucStaticallyAllocated);
        let addressSend                 = await this.helperGetListOfAddressesInListObj(xTasksWaitingToSend);
        //let userViewSend              = await this.getTaskAddressesAndNames(addressSend);
        let addressRec                  = await this.helperGetListOfAddressesInListObj(xTasksWaitingToReceive);
        //let userViewRec               = await this.getTaskAddressesAndNames(addressRec);
        map["blockedAddresses"]         = addressSend.concat(addressRec);
        let blockedTasks                = xTasksWaitingToReceive.uxNumberOfItems + xTasksWaitingToSend.uxNumberOfItems;
        map["blockedTasks"]             = blockedTasks > 0 ? blockedTasks : "-";
        map["uxItemSize"]               = uxItemSize; 
        // map["CRxLock"]               = CRxLock >= 0 ? CRxLock : "Not Locked";
        // map["CTxLock"]               = CTxLock >= 0 ? CTxLock : "Not Locked";
        map["xMutexHolder"]             = unionMap["xMutexHolder"];
        map["uxRecursiveCallCount"]     = unionMap["uxRecursiveCallCount"];
        map["pcTail"]                   = '0x' + Number(unionMap["pcTail"]).toString(16);
        map["pcReadFrom"]               = '0x' + Number(unionMap["pcReadFrom"]).toString(16);
        map["pcHead"]                   = '0x' + pcHead.toString(16);
        map["pcWriteTo"]                = '0x' + pcWriteTo.toString(16); 
    
        return map;
    }

    async fillQueueObjectFromMap(map, view){
        if (map["Type"] != "Queue") {
            return;
        }
    
        let queue = new QueueInstance();
        queue.Name                  = map["Name"];
        queue.Address               = map["Address"];
        queue.Length                = map["uxLength"];
        queue.MsgWaiting            = map["uxMessagesWaiting"];
        queue.ItemSize              = map["uxItemSize"];
        queue.TasksPendingSend      = map["TasksWaitingToSend"];
        queue.TasksPendingReceive   = map["TasksWaitingToReceive"];
        queue.BlockedTasks          = map["blockedTasks"];
        queue.StorageStart          = map["pcHead"];
        queue.StorageEnd            = map["pcTail"];
    
        view.push(queue);
    }

    async getSemaphoreInstances() {
        const table = [];

        let queueMaps = await this.createMapForEachQueueObject();
        //let queueMaps = null;
        /* queueMaps will be null here when configQUEUE_REGISTRY_SIZE
        * is <= 0 thus we add a message to guide the user */
        if (queueMaps == null) {
            let message = new SemaphoreInstance();
            message.Address = "Set configQUEUE_REGISTRY_SIZE to the maximum number of queues + semaphores + mutexes that ROV should be able to read";
            table.push(message);
            return (table);
        }

        for (let i = 0; i < queueMaps.length; i++) {
            await this.fillSemaphoreObjectFromMap(queueMaps[i], table);
        }

        table.sort(compareAddress);
        return (table);
    }

    async fillSemaphoreObjectFromMap(map, view){
        if (map["Type"] != "Semaphore (Counting)" && map["Type"] != "Semaphore (Binary)") {
            return;
        }
    
        let sem = new SemaphoreInstance();
        sem.Name             = map["Name"];
        sem.Type             = map["Type"];
        sem.Address          = map["Address"];
        sem.MaxCnt           = map["uxLength"];
        sem.Available        = map["uxMessagesWaiting"];
        sem.BlockedCount     = map["TasksWaitingToReceive"];
        sem.StaticallyAlloc  = map["ucStaticallyAllocated"];
        sem.BlockedTasks     = map["blockedTasks"];

        view.push(sem);
    }
    
    async getMutexInstances() {
        const table = [];

        let queueMaps = await this.createMapForEachQueueObject();
        //let queueMaps = null;
        /* queueMaps will be null here when configQUEUE_REGISTRY_SIZE
        * is <= 0 thus we add a message to guide the user */
        if (queueMaps == null) {
            let message = new MutexInstance();
            message.Address = "Set configQUEUE_REGISTRY_SIZE to the maximum number of queues + semaphores + mutexes that ROV should be able to read";
            table.push(message);
            return (table);
        }

        for (let i = 0; i < queueMaps.length; i++) {
            await this.fillMutexObjectFromMap(queueMaps[i], table);
        }

        table.sort(compareAddress);
        return (table);
    }

    async fillMutexObjectFromMap(map, view){
        if (map["Type"] != "Mutex" && map["Type"] != "Mutex (Recursive)") {
            return;
        }

        let mutex = new MutexInstance();
        mutex.Name              = map["Name"];
        mutex.Address           = map["Address"];
        mutex.MaxCnt            = map["uxLength"];
        mutex.Type              = map["Type"];
        mutex.Available         = map["uxMessagesWaiting"];
        mutex.BlockedCount      = map["TasksWaitingToReceive"];
        mutex.Holder            = map["xMutexHolder"];            
        mutex.BlockedTasks      = map["blockedTasks"];
        mutex.StaticallyAlloc   = map["ucStaticallyAllocated"];

        view.push(mutex);
    }

    getModuleName() {
        return 'FreeRTOS';
    }
}

// Helper functions

function compareAddress(a, b) {
    return +(a.Address ?? 0) - +(b.Address ?? 0);
}

function compareHandle(a, b) {
    return +(a.Handle ?? 0) - +(b.Handle ?? 0);
}

function checkIfTaskIsBlockingOnQueue(taskAddress, queueMap){
    let blocked = queueMap["blockedAddresses"];
    for (let i = 0; i < blocked.length; i++) {
        if (String(blocked[i]) == String(taskAddress)) {
            return true;
        }
    }
    return false;
}

function getQueueType(ucQueueType){
    switch (ucQueueType) {
        default:
        case 0:
            return "Queue";
        case 1:
            return "Mutex";
        case 2:
            return "Semaphore (Counting)";
        case 3:
            return "Semaphore (Binary)";
        case 4:
            return "Mutex (Recursive)";
    }
}

// For debugging
function propsAsString(obj) {
    return Object.keys(obj).map(function(k) { return k + ":" + obj[k] }).join(" AND ")
}

exports.classCtor = FreeRTOS;
