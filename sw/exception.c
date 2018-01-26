/*
 * If a serious error occurs, one of the fault
 * exception vectors in this file will be called.
 *
 * This file attempts to aid the unfortunate debugger
 * to blame someone for the crashing code
 *
 *  Created on: 12.06.2013
 *      Author: uli
 *
 * Released under the CC0 1.0 Universal (public domain)
 */
#include <stdint.h>
#include <ch.h>
#include <string.h>
#include <hal.h>

// Can be uncommented to see backtrace.
#define DEBUG_BACKTRACE

/**
 * Executes the BKPT instruction that causes the debugger to stop.
 * If no debugger is attached, this will be ignored
 */
#define bkpt() __asm volatile("BKPT #0\n")

//See http://infocenter.arm.com/help/topic/com.arm.doc.dui0552a/BABBGBEC.html
typedef enum  {
    Reset = 1,
    NMI = 2,
    HardFault = 3,
    MemManage = 4,
    BusFault = 5,
    UsageFault = 6,
} FaultType;

/* On hard fault, copy FAULT_PSP to the sp reg so gdb can give a trace */
void **FAULT_PSP;
register void *stack_pointer asm("sp");

void HardFault_Handler(void) {
#ifdef DEBUG_BACKTRACE
    asm("mrs %0, psp" : "=r"(FAULT_PSP) : :);
    stack_pointer = FAULT_PSP;
    // Here will be good to assert Error LED, like
    // GPIOA->ODR |= 1;
    while(1);
#else
    //Copy to local variables (not pointers) to allow GDB "i loc" to directly show the info
    //Get thread context. Contains main registers including PC and LR
    struct port_extctx ctx;

    memcpy(&ctx, (void*)__get_PSP(), sizeof(struct port_extctx));
    //Interrupt status register: Which interrupt have we encountered, e.g. HardFault?
    FaultType hf_faultType = (FaultType)__get_IPSR();
    (void)hf_faultType;
    //For HardFault/BusFault this is the address that was accessed causing the error

    // This is to prevent variables to be optimized out
    if ((ctx.r0 != ctx.r1) && (ctx.xpsr != ctx.pc))
        asm("nop");
    bkpt();
    NVIC_SystemReset();
#endif
}

void BusFault_Handler(void) {
  bkpt();
}

void UsageFault_Handler(void) {
#ifdef DEBUG_BACKTRACE
    asm("mrs %0, psp" : "=r"(FAULT_PSP) : :);
    stack_pointer = FAULT_PSP;
    while(1);
#else
    //Copy to local variables (not pointers) to allow GDB "i loc" to directly show the info
    //Get thread context. Contains main registers including PC and LR
    struct port_extctx ctx;
    memcpy(&ctx, (void*)__get_PSP(), sizeof(struct port_extctx));
    (void)ctx;
    //Interrupt status register: Which interrupt have we encountered, e.g. HardFault?
    FaultType faultType = (FaultType)__get_IPSR();
    (void)faultType;
    bkpt();
    NVIC_SystemReset();
#endif
}

void MemManage_Handler(void) {
#ifdef DEBUG_BACKTRACE
    asm("mrs %0, psp" : "=r"(FAULT_PSP) : :);
    stack_pointer = FAULT_PSP;
    while(1);
#else
   //Copy to local variables (not pointers) to allow GDB "i loc" to directly show the info
    //Get thread context. Contains main registers including PC and LR
    struct port_extctx ctx;
    memcpy(&ctx, (void*)__get_PSP(), sizeof(struct port_extctx));
    (void)ctx;
    //Interrupt status register: Which interrupt have we encountered, e.g. HardFault?
    FaultType faultType = (FaultType)__get_IPSR();
    (void)faultType;
    //For HardFault/BusFault this is the address that was accessed causing the error
    bkpt();
    NVIC_SystemReset();
#endif
}
