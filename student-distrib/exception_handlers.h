#ifndef _EXCEPTION_HANDLERS_H
#define _EXCEPTION_HANDLERS_H

/* Exception index that we do not use because it is reserved */
#define RESERVED_EXCEPTION_INDEX 15
/* Number of exception handlers that we implement */
#define NUM_EXCEPTION_HANDLERS   20

/* Indices of interrupt handlers in exception_handlers array */
#define DIVIDE_ZERO_E                 0
#define DEBUG_E                       1
#define NMINTERRUPT_E                 2
#define BREAKPOINT_E                  3
#define OVERFLOW_E                    4
#define BOUND_RANGE_EXCEEDED_E        5
#define INVALID_OPCODE_E              6
#define DEVICE_NA_E                   7
#define DOUBLE_FAULT                  8
#define COPROCESSOR_SEGMENT_OVERRUN_E 9
#define INVALID_TSS_E                 10
#define SEGMENT_NP_E                  11
#define STACK_SEGMENT_FAULT_E         12
#define GENERAL_PROTECTION_E          13
#define PAGE_FAULT_E                  14
/* 15 is missing because it is reserved */
#define FLOATING_POINT_ERROR_E        16
#define ALIGNMENT_CHECK_E             17
#define MACHINE_CHECK_E               18
#define FLOATING_POINT_EXCEPTION_E    19

/* Array of function pointers to exception handlers */
extern uint32_t exception_handlers[NUM_EXCEPTION_HANDLERS];

// Assembly linkages for all the exception handlers
extern void divide_zero_linkage();
extern void debug_linkage();
extern void nminterrupt_linkage();
extern void breakpoint_linkage();
extern void overflow_linkage();
extern void bound_range_exceeded_linkage();
extern void invalid_opcode_linkage();
extern void device_na_linkage();
extern void double_fault_linkage();
extern void coprocessor_segment_overrun_linkage();
extern void invalid_tss_linkage();
extern void segment_np_linkage();
extern void stack_segment_fault_linkage();
extern void general_protection_linkage();
extern void page_fault_linkage();
extern void floating_point_error_linkage();
extern void alignment_check_linkage();
extern void machine_check_linkage();
extern void floating_point_exception_linkage();

#endif
