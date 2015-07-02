;;  Copyright ARM Ltd 2005-2008. All rights reserved.
;;

;==================================================================
; Cortex-A8 Embedded example - Startup Code
;==================================================================

        PRESERVE8
        AREA   INITARMv6v7, CODE, READONLY

        ENTRY

; Standard definitions of mode bits and interrupt (I & F) flags in PSRs

Mode_USR        EQU     0x10
Mode_FIQ        EQU     0x11
Mode_IRQ        EQU     0x12
Mode_SVC        EQU     0x13
Mode_ABT        EQU     0x17
Mode_UNDEF      EQU     0x1B
Mode_SYS        EQU     0x1F

I_Bit           EQU     0x80 ; when I bit is set, IRQ is disabled
F_Bit           EQU     0x40 ; when F bit is set, FIQ is disabled

        EXPORT  Start
Start

;==================================================================
; if MMU/MPU enabled - disable (useful for ARMulator tests)
;==================================================================

        MRC     p15, 0, r0, c1, c0, 0       ; read CP15 register 1
        BIC     r0, r0, #0x1                ; clear bit 0
        MCR     p15, 0, r0, c1, c0, 0       ; write value back

;==================================================================
; Initialise Supervisor Mode Stack
; Note stack must be 8 byte aligned.
;==================================================================

        IMPORT  ||Image$$ARM_LIB_STACKHEAP$$ZI$$Limit||   ; Get Stack from scatter file

        LDR     SP, =||Image$$ARM_LIB_STACKHEAP$$ZI$$Limit||
        
;===================================================================
; MMU Configuration
;
; Configure system to use extended v6 format pagetables
; Set translation table base
;===================================================================
        IMPORT ||Image$$TTB$$ZI$$Base||  

        ; Specify v6 format pagetables with no subpages
        ; set bit 23 [XP] in CP15 control register.
        ; set bit 22 [U] for unaligned access

        MRC     p15,0,r0,c1,c0,0
        BIC     r0,r0, #2
        LDR     r1, =0xC00000
        ORR     r0,r0,r1
        MCR     p15,0,r0,c1,c0,0
        
        ; Invalidate Branch prediction array
        MOV    r0,#0
        MCR    p15, 0, r0, c7, c5, 6
           
        ; Cortex-A8 I-TLB and D-TLB invalidation
        MOV    r0,#0
        MCR    p15, 0, r0, c8, c7, 0 
        
        ; Invalidate L1 Instruction Cache
        MOV r0, #0
        MCR p15, 0, r0, c7, c5, 0 ; Invalidate Instruction Cache
        
        ; v6 processors supports complete data invalidation
        MRC p15, 0, r0, c0, c0, 0        
        MRC p15, 0, r1, c0, c2, 2  ; Instruction Feature register
        
        LSR r0,r0,#0x10
        AND r0,r0,#0xF   
        CMP r0, #0xF
        BNE v6code
        ; else if ( (IFR2 & 0xF0) == 0x30 )
        AND r1,r1,#0xF0
        CMP r1, #0x30
        BEQ v7code
v6code        
        
        MOV r0, #0
        MCR p15, 0, r0, c7, c6, 0 
        
        B Finished

v7code        
        ; Invalidate Data/Unified Caches
        
        MRC p15, 1, r0, c0, c0, 1   ; Read CLIDR
        ANDS r3, r0, #&7000000
        MOV r3, r3, LSR #23         ; Total cache levels << 1
        BEQ Finished
    
        MOV r10, #0                 ; R10 holds current cache level << 1
Loop1   ADD r2, r10, r10, LSR #1    ; R2 holds cache "Set" position 
        MOV r1, r0, LSR r2          ; Bottom 3 bits are the Cache-type for this level
        AND r1, R1, #7              ; Get those 3 bits alone
        CMP r1, #2
        BLT Skip                    ; No cache or only instruction cache at this level
        
        MCR p15, 2, r10, c0, c0, 0  ; Write the Cache Size selection register
        MOV r1, #0
        MCR p15, 0, r1, c7, c5, 4   ; PrefetchFlush to sync the change to the CacheSizeID reg
        MRC p15, 1, r1, c0, c0, 0   ; Reads current Cache Size ID register
        AND r2, r1, #&7             ; Extract the line length field
        ADD r2, r2, #4              ; Add 4 for the line length offset (log2 16 bytes)
        LDR r4, =0x3FF
        ANDS r4, r4, r1, LSR #3     ; R4 is the max number on the way size (right aligned)
        CLZ r5, r4                  ; R5 is the bit position of the way size increment
        LDR r7, =0x00007FFF
        ANDS r7, r7, r1, LSR #13    ; R7 is the max number of the index size (right aligned)

Loop2   MOV r9, r4                  ; R9 working copy of the max way size (right aligned)

Loop3   ORR r11, r10, r9, LSL r5    ; Factor in the Way number and cache number into R11
        ORR r11, r11, r7, LSL r2    ; Factor in the Set number
        MCR p15, 0, r11, c7, c6, 2  ; Invalidate by set/way
        SUBS r9, r9, #1             ; Decrement the Way number
        BGE Loop3
        SUBS r7, r7, #1             ; Decrement the Set number
        BGE Loop2
Skip    ADD r10, r10, #2            ; increment the cache number
        CMP r3, r10
        BGT Loop1
        
Finished

        ; v6 supports two translation tables
        ; Configure translation table base (TTB) control register cp15,c2
        ; to a value of all zeros, indicates we are using TTB register 0.

        MOV     r0,#0x0
        MCR     p15, 0, r0, c2, c0, 2
        
        ; write the address of our page table base to TTB register 0.

        LDR     r0,=||Image$$TTB$$ZI$$Base||
        MCR     p15, 0, r0, c2, c0, 0   


;===================================================================
; PAGE TABLE generation 
;===================================================================

        IMPORT TTB_generate
        BL TTB_generate


;==================================================================
; Enable access to NEON/VFP by enabling access to Coprocessors 10 and 11.
; Enables Full Access i.e. in both priv and non priv modes
;==================================================================
 
	; PRACTICAL 9.1.1
	; -------------
        ; We have to enable the NEON co-processors CP10 and CP11
        ; before NEON instructions can be executed
        ;
        ; TODO: Refer to the Coprocessor Access Control Register 
        ;       in the Cortex-A8 TRM and enable CP10 and CP11 using
        ;       a MRC/MCR instruction sequence  
   
        
        MRC p15, 0, r0 , c1, c0, 2
        ORR r0, r0, #0xF00000          
        MCR p15, 0, r0 , c1, c0, 2 
    

;==================================================================
; Switch on the VFP and Neon Hardware
;=================================================================

        ; PRACTICAL 9.1.1
	; -------------
        ; Once the co-processors for NEON have been enabled, 
        ; NEON must be explicitly enabled in the Floating-point 
        ; exception Register, FPEXC
        ;     
        ; TODO: Refer to the Floating-point exception Register (FPEXC)
        ;       in the Cortex-A8 TRM and enable NEON. The FPEXC
        ;       register is accessed using the VMSR instruction.
        ;       Refer to the ARM ARM for instruction details.     
        
        LDR r0, =0x40000000
        VMSR FPEXC, r0
        
;===================================================================        
; Setup domain control register
;===================================================================

        ; Enable all domains to client mode
        MRC     p15, 0, r0, c3, c0, 0     ; Read Domain Access Control Register
        LDR     r0, =0x55555555           ; This initialize every domain entry to b01 (client)
        MCR     p15, 0, r0, c3, c0, 0     ; Write Domain Access Control Register          
        
        
;===================================================================
; Enable MMU and Branch to __main
;===================================================================

        IMPORT  __main                      ; before MMU enabled import label to __main
        

        MRC     p15, 0, r0, c1, c0, 0       ; read CP15 register 1
        ORR     r0, r0, #0x1                ; enable MMU before scatter loading
        MCR     p15, 0, r0, c1, c0, 0       ; write CP15 register 1

        LDR     pc,=__main                         ; branch to __main  C library entry point

        END
