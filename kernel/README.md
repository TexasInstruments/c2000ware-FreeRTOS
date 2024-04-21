<div align="center">

<img src="https://upload.wikimedia.org/wikipedia/commons/b/ba/TexasInstruments-Logo.svg" width="150"><br/>
# c2000ware-rtos

[Introduction](#introduction) | [Layout](#repositorylayout) | [Overview](#overview) | [Contribute](#contributing-to-the-project)

</div>

## Introduction
FreeRTOS is a market-leading real-time operating system (RTOS) for microcontrollers and small microprocessors.
This repository contains FreeRTOS kernel source/header files and kernel ports and demos. 

C2000Ware SDK supports FreeRTOS on C28x and Cortex-M4(CM) cores.

List of devices supported
- [F280015x](https://www.ti.com/product/TMS320F2800157) 
- [F280013x](https://www.ti.com/product/TMS320F2800137)
- [F28003x](https://www.ti.com/product/TMS320F280039C)
- [F28002x](https://www.ti.com/product/TMS320F280025C)
- [F2838x](https://www.ti.com/product/TMS320F28388D).
- [F28P65x](https://www.ti.com/product/TMS320F28P650DK).
- [F28P55x](https://www.ti.com/product/TMS320F28P559SJ-Q1).


## Repository Layout
- kernel/FreeRTOS/Source 
	- This folder contains the source code for the kernel 
- kernel/FreeRTOS/Source/portable/CCS/C2000_C28x 
    -  This folder contains the port files for C28x .
- kernel/FreeRTOS/Source/portable/CCS/ARM_CM4 
    -  This folder contains the port files for Cortex-M4.	.
- kernel/FreeRTOS/rov
	- This folder contains the Runtime Object View (ROV) files. ROV is supported only on CCS Theia IDE. 
- kernel/FreeRTOS/Demo
	- This folder contains the demo application projects 
	


## Overview

	This repository has a dependency on c2000ware-core-sdk repository. It is added as a git submodule to the c2000ware-core-sdk repository.
	
## Contributing to the project

This project is currently not accepting contributions. 	