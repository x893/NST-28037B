; Amount of memory (in bytes) allocated for Stack
; Tailor this value to your application needs
; <h> Stack Configuration
;	<o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size		EQU		0x00000400
				AREA	STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem		SPACE	Stack_Size
__initial_sp


; <h> Heap Configuration
;	<o>	Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size		EQU		0x00000000
				AREA	HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem		SPACE	Heap_Size
__heap_limit

				PRESERVE8
				THUMB

; Vector Table Mapped to Address 0 at Reset
				AREA	RESET, DATA, READONLY
				EXPORT	__Vectors
				EXPORT	__Vectors_End
				EXPORT	__Vectors_Size

__Vectors		DCD		__initial_sp		; Top of Stack
				DCD		Reset_Handler		; Reset Handler
				DCD		NMI_Handler			; NMI Handler
				DCD		HardFault_Handler	; Hard Fault Handler
				DCD		MemManage_Handler	; MPU Fault Handler
				DCD		BusFault_Handler	; Bus Fault Handler
				DCD		UsageFault_Handler	; Usage Fault Handler
__Vectors_End
__Vectors_Size	EQU	__Vectors_End - __Vectors

				AREA	|.text|, CODE, READONLY

; Reset handler
Reset_Handler	PROC

				EXPORT	Reset_Handler			 [WEAK]
				IMPORT	__main
				LDR		R0, =__main
				BX		R0

				ENDP

; Dummy Exception Handlers (infinite loops which can be modified)
FatalHandler	PROC
				EXPORT	NMI_Handler			[WEAK]
				EXPORT	HardFault_Handler	[WEAK]
				EXPORT	MemManage_Handler	[WEAK]
				EXPORT	BusFault_Handler	[WEAK]
				EXPORT	UsageFault_Handler	[WEAK]
NMI_Handler
HardFault_Handler
MemManage_Handler
BusFault_Handler
UsageFault_Handler
				B		.
				ENDP

				ALIGN

;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
				 IF		:DEF:__MICROLIB
				
				 EXPORT	__initial_sp
				 EXPORT	__heap_base
				 EXPORT	__heap_limit
				
				 ELSE
				
				 IMPORT	__use_two_region_memory
				 EXPORT	__user_initial_stackheap
				 
__user_initial_stackheap

				 LDR	R0, =	Heap_Mem
				 LDR	R1, = (Stack_Mem + Stack_Size)
				 LDR	R2, = (Heap_Mem +	Heap_Size)
				 LDR	R3, = Stack_Mem
				 BX		LR

				 ALIGN

				 ENDIF

				 END

;******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE*****
