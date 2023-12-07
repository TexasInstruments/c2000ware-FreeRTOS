<div align="center">

<img src="https://upload.wikimedia.org/wikipedia/commons/b/ba/TexasInstruments-Logo.svg" width="150"><br/>
# c2000ware-FreeRTOS

[Introduction](#introduction) | [Layout](#repositorylayout) | [Note](#note) | [Contribute](#contributing-to-the-project)

</div>

## Introduction
FreeRTOS is a market-leading real-time operating system (RTOS) for microcontrollers and small microprocessors.
This repository contains FreeRTOS kernel source/header files and kernel ports only. This repository is referenced as a submodule in c2000ware-core-sdk repository.

C2000Ware SDK supports FreeRTOS on C28x and Cortex-M4(CM) cores.

List of devices supported
- [F280015x](https://www.ti.com/product/TMS320F2800157) 
- [F280013x](https://www.ti.com/product/TMS320F2800137)
- [F28003x](https://www.ti.com/product/TMS320F280039C)
- [F28002x](https://www.ti.com/product/TMS320F280025C)
- [F2838x](https://www.ti.com/product/TMS320F28388D)
- [F28P65x](https://www.ti.com/product/TMS320F28P650DK).


## Repository Layout
- kernel/FreeRTOS/Source 
	- This folder contains the source code for the kernel 
- kernel/FreeRTOS/Source/portable/CCS/C2000_C28x 
    -  This folder contains the port files for C28x .
- kernel/FreeRTOS/Source/portable/CCS/ARM_CM4 
    -  This folder contains the port files for Cortex-M4.	.
- kernel/FreeRTOS/Demo
	- This folder contains the demo application projects 
	
## Note
   This repository has a dependency on [c2000ware-core-sdk](https://github.com/TexasInstruments/c2000ware-core-sdk) repository. 
   Please clone the c2000ware-core-sdk repo and then clone this repo. 
   The contents of this repo (kernel) folder has to be copied into the c2000ware-core-sdk repo.

	
## Contributing to the project

This project is currently not accepting contributions. 	
