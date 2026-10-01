;-------------------------------------------------------------------------------------------------
;/*
; * FreeRTOS Kernel <DEVELOPMENT BRANCH>
; * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
; *
; * SPDX-License-Identifier: MIT
; *
; * Permission is hereby granted, free of charge, to any person obtaining a copy of
; * this software and associated documentation files (the "Software"), to deal in
; * the Software without restriction, including without limitation the rights to
; * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
; * the Software, and to permit persons to whom the Software is furnished to do so,
; * subject to the following conditions:
; *
; * The above copyright notice and this permission notice shall be included in all
; * copies or substantial portions of the Software.
; *
; * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
; * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
; * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
; * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
; * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
; *
; * https://www.FreeRTOS.org
; * https://github.com/FreeRTOS
; *
; */
;-------------------------------------------------------------------------------------------------

  .cdecls C,LIST, "portdefines.h"

  .if __TI_EABI__
  .asg    pxCurrentTCB, _pxCurrentTCB
  .asg    ulCriticalNesting, _ulCriticalNesting
  .asg    xTaskIncrementTick, _xTaskIncrementTick
  .asg    vTaskSwitchContext, _vTaskSwitchContext
  
  .asg    portGET_HIGHEST_PRIORITY, _getHighestPriority
  .asg    getSTF, _getSTF
  .asg    portRESTORE_FIRST_CONTEXT, _portRESTORE_FIRST_CONTEXT
  .asg    portTICK_ISR, _portTICK_ISR
  .asg    portYIELD_ISR, _portYIELD_ISR
  .endif

  .ref _pxCurrentTCB
  .ref _ulCriticalNesting
  .ref _xTaskIncrementTick
  .ref _vTaskSwitchContext

  .def _portTICK_ISR
  .def _portYIELD_ISR
  .def _portRESTORE_FIRST_CONTEXT
  .def _getHighestPriority

;=================================================================================================
; Macros
;=================================================================================================

;-------------------------------------------------------------------------------------------------
; SAVE_CONTEXT
;-------------------------------------------------------------------------------------------------

SAVE_CONTEXT .macro
; Stash IER in ACC before align
  MOV     AL, *-SP[5]
  ASP
  .if ((.TMS320C2800_FPU32 + .TMS320C2800_FPU64) >= 1)
  PUSH    RB
  .endif
  PUSH    AR1H:AR0H
  PUSH    RPC
  MOVL    *SP++, XT
  MOVL    *SP++, XAR2
  MOVL    *SP++, XAR3
  MOVL    *SP++, XAR4
  MOVL    *SP++, XAR5
  MOVL    *SP++, XAR6
  MOVL    *SP++, XAR7
  .if (.TMS320C2800_FPU64 = 1)
  MOV32   *SP++, STF
  MOV32   *SP++, R0L
  MOV32   *SP++, R0H
  MOV32   *SP++, R1L
  MOV32   *SP++, R1H
  MOV32   *SP++, R2L
  MOV32   *SP++, R2H
  MOV32   *SP++, R3L
  MOV32   *SP++, R3H
  MOV32   *SP++, R4L
  MOV32   *SP++, R4H
  MOV32   *SP++, R5L
  MOV32   *SP++, R5H
  MOV32   *SP++, R6L
  MOV32   *SP++, R6H
  MOV32   *SP++, R7L
  MOV32   *SP++, R7H
  .elseif (.TMS320C2800_FPU32 = 1)
  MOV32   *SP++, STF
  MOV32   *SP++, R0H
  MOV32   *SP++, R1H
  MOV32   *SP++, R2H
  MOV32   *SP++, R3H
  MOV32   *SP++, R4H
  MOV32   *SP++, R5H
  MOV32   *SP++, R6H
  MOV32   *SP++, R7H
  .endif
  PUSH    DP:ST1
  PUSH    ACC
  .endm

;-------------------------------------------------------------------------------------------------
; RESTORE_CONTEXT
;-------------------------------------------------------------------------------------------------

RESTORE_CONTEXT .macro
  POP     ACC
  POP     DP:ST1
  .if (.TMS320C2800_FPU64 = 1)
  MOV32   R7H, *--SP
  MOV32   R7L, *--SP
  MOV32   R6H, *--SP
  MOV32   R6L, *--SP
  MOV32   R5H, *--SP
  MOV32   R5L, *--SP
  MOV32   R4H, *--SP
  MOV32   R4L, *--SP
  MOV32   R3H, *--SP
  MOV32   R3L, *--SP
  MOV32   R2H, *--SP
  MOV32   R2L, *--SP
  MOV32   R1H, *--SP
  MOV32   R1L, *--SP
  MOV32   R0H, *--SP
  MOV32   R0L, *--SP
  MOV32   STF, *--SP
  .elseif (.TMS320C2800_FPU32 = 1)
  MOV32   R7H, *--SP
  MOV32   R6H, *--SP
  MOV32   R5H, *--SP
  MOV32   R4H, *--SP
  MOV32   R3H, *--SP
  MOV32   R2H, *--SP
  MOV32   R1H, *--SP
  MOV32   R0H, *--SP
  MOV32   STF, *--SP
  .endif
  MOVL    XAR7, *--SP
  MOVL    XAR6, *--SP
  MOVL    XAR5, *--SP
  MOVL    XAR4, *--SP
  MOVL    XAR3, *--SP
  MOVL    XAR2, *--SP
  MOVL    XT, *--SP
  POP     RPC
  POP     AR1H:AR0H
  .if ((.TMS320C2800_FPU32 + .TMS320C2800_FPU64) >= 1)
  POP     RB
  .endif
  NASP
; Update IER in restored context
  MOV     *-SP[5], AL
  .endm

;-------------------------------------------------------------------------------------------------
; SWAP_TASK_CONTEXT
;-------------------------------------------------------------------------------------------------

SWAP_TASK_CONTEXT .macro
; Retrieve IER from saved context
  POP     XAR7

; Save critical section nesting counter
  MOVL    XAR0, #_ulCriticalNesting
  MOVL    ACC, *XAR0
  PUSH    ACC

; Save stack pointer in current TCB
  MOVL    XAR0, #_pxCurrentTCB
  MOVL    XAR0, *XAR0
  MOVL    XAR6, #0              ; Set to 0 before moving the new value of pxTopOfStack
  MOV     AR6, @SP
  MOVL    *XAR0, XAR6

; Save IER on top of stack
  PUSH    XAR7

; Select the next task
  LCR     _vTaskSwitchContext

; Retrieve new IER value
  POP     XAR7
  
; Restore stack pointer from new TCB
  MOVL    XAR0, #_pxCurrentTCB
  MOVL    XAR0, *XAR0
  MOVL    XAR0, *XAR0
  MOV     @SP, AR0

; Restore critical section nesting counter
  MOVL    XAR0, #_ulCriticalNesting
  POP     ACC
  MOVL    *XAR0, ACC

; Replace IER into initial position
  PUSH    XAR7
  .endm

;=================================================================================================
; Public functions
;=================================================================================================

;-------------------------------------------------------------------------------------------------
; _getHighestPriority
;-------------------------------------------------------------------------------------------------

_getHighestPriority:
  CSB     ACC         ; CSB returns (Num_leading_zeroes - 1) in T register 
  MOV     ACC, #30
  SUB     ACC, T
  LRETR

;-------------------------------------------------------------------------------------------------
; _getSTF
;-------------------------------------------------------------------------------------------------

  .if ((.TMS320C2800_FPU32 + .TMS320C2800_FPU64) >= 1)
  .def _getSTF

_getSTF:
  MOV32   *SP++, STF
  POP     ACC
  LRETR
  .endif

;-------------------------------------------------------------------------------------------------
; _portRESTORE_FIRST_CONTEXT
;-------------------------------------------------------------------------------------------------

_portRESTORE_FIRST_CONTEXT:
; Restore stack pointer from new TCB
  MOVL    XAR0, #_pxCurrentTCB
  MOVL    XAR0, *XAR0
  MOVL    XAR0, *XAR0
  MOV     @SP, AR0

; Restore XAR4 and RPC from saved task stack.
; and return to main task function.
; SP should be set to stack start plus 2 before LRETR.
  .if (.TMS320C2800_FPU64 = 1)
  SUBB   SP, #44
  .elseif (.TMS320C2800_FPU32 = 1)
  SUBB   SP, #28
  .else
  SUBB   SP, #10
  .endif
  POP    XAR4

  .if ((.TMS320C2800_FPU32 + .TMS320C2800_FPU64) >= 1)
  SUBB   SP, #14
  .else
  SUBB   SP, #12
  .endif
  POP    RPC
  SUBB   SP, #10
  LRETR

;-------------------------------------------------------------------------------------------------
; _portTICK_ISR
;-------------------------------------------------------------------------------------------------

_portTICK_ISR:

  SAVE_CONTEXT

; Clear timer overflow
  MOVL    XAR0, #PORT_TICK_TIMER_O_TCR
  MOV     AL, *XAR0
  OR      AL, #0x8000
  MOV     *XAR0, AL

; Increment tick counter
  LCR     _xTaskIncrementTick

; Perform context switch if preempted, otherwise skip
  CMPB    AL, #0x0
  SB      SKIP_CONTEXT_SWITCH, EQ
  SWAP_TASK_CONTEXT

SKIP_CONTEXT_SWITCH:
  RESTORE_CONTEXT

  IRET

;-------------------------------------------------------------------------------------------------
; _portYIELD_ISR
;-------------------------------------------------------------------------------------------------

_portYIELD_ISR:
 
  SAVE_CONTEXT

; ACK the PIE interrupt
  MOV     @AL, #PORT_PIE_ACK_YIELD
  MOV     *(0:0x0CE1), @AL

; Perform context switch
  SWAP_TASK_CONTEXT

  RESTORE_CONTEXT

  IRET
