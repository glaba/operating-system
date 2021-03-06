#ifndef _PROCESSES_H
#define _PROCESSES_H

#include "types.h"
#include "system_calls.h"
#include "system_call_linkage.h"
#include "keyboard.h"
#include "dynamic_array.h"
#include "list.h"
#include "spinlock.h"
#include "signals.h"

// Uncomment PROC_DEBUG_ENABLE to enable debugging
// #define PROC_DEBUG_ENABLE
#ifdef PROC_DEBUG_ENABLE
	#define PROC_DEBUG(f, ...) printf(f, ##__VA_ARGS__)
#else
	#define PROC_DEBUG(f, ...) // Nothing
#endif

// The number of text-based TTYs (1-based indices)
#define NUM_TEXT_TTYS 3
// The total number of TTYs
#define NUM_TTYS 4

// Magic number that must appear in the first 4 bytes of all executables
#define ELF_MAGIC 0x464C457F
// The virtual address that all processes' data is mapped to
#define EXECUTABLE_VIRT_PAGE_START 0x8000000
// The offset within a page that an executable should be copied to
#define EXECUTABLE_PAGE_OFFSET 0x48000
// The offset in the executable where the entrypoint of the program is stored
#define ENTRYPOINT_OFFSET 24

// Bitmask for ESP that will yield the top of the kernel stack
#define KERNEL_STACK_BASE_BITMASK 0xFFFFE000

// The maximum number of files that can be open for a process
// This limit only exists to prevent userspace programs from using up too much kernel memory
#define MAX_NUM_FILES 64

// The static file descriptors assigned to stdin and stdout for all programs
#define STDIN  0
#define STDOUT 1
#define MOUSE_FD 2
#define UDP_FD 3

extern void *vid_mem_buffers[NUM_TTYS];

typedef struct fops_t {
	int32_t (*open )(const uint8_t*);
	int32_t (*close)(int32_t);
	int32_t (*read )(int32_t fd, void *buf, int32_t bytes);
	int32_t (*write)(int32_t fd, const void *buf, int32_t bytes);
} fops_t;

typedef struct file_t {
	// Stores pointer to open/close/read/write
	fops_t* fd_table;
	// The inode number of the file (only valid for data file, should be 0 for directories)
	uint32_t inode;
	// The current position in the file
	uint32_t file_pos;
	// Set to 1 if this file array entry is in use and 0 if not
	uint32_t in_use;
} file_t;

// A dynamic array containing file_t elements
typedef DYNAMIC_ARRAY(file_t, file_dyn_arr) file_dyn_arr;

// Represents a page mapping
typedef struct page_mapping {
	// The index of the virtual address of the page (corresponding to 4MB jumps per index)
	int virt_index;
	// The index of the physical address of the page (corresponding to 4MB jumps per index)
	int phys_index;
} page_mapping;

// A dynamic array of ints where each item represents an index of a 4MB page used by a process
typedef DYNAMIC_ARRAY(page_mapping, page_mapping_dyn_arr) page_mapping_dyn_arr;

// States that a process can take on
// In this state, the process is running normally and will get scheduled
#define PROCESS_RUNNING  0
// In this state, the process is blocking on some system call, and will be scheduled when its state
//  is switched back to RUNNING
#define PROCESS_SLEEPING 1
// In this state, the process is marked for deletion, and when its turn comes in the scheduler,
//  all its resources will be deleted and it will not run
#define PROCESS_STOPPING 2

// All the registers that a process may be using before being interrupted that should be restored
struct process_context {
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t eax;
	uint32_t ds;
	uint32_t es;
	uint32_t fs;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;
} __attribute__((packed));

typedef struct process_context process_context;

// The order of the items in this struct are NOT to be changed
struct scheduler_context {
	uint32_t esp;
	uint32_t ebp;
	uint32_t eip; // Should be inside the kernel code
} __attribute__((packed));

typedef struct scheduler_context scheduler_context;

struct blocking_call_t {
	// The type of blocking call that the process is currently in
	uint32_t type;
	// Any data returned by the blocking call (may be interpreted as a pointer to some struct
	//  or a 4 byte value or nothing at all, depending on the type of the blocking call)
	uint32_t data;
} __attribute__((packed));

typedef struct blocking_call_t blocking_call_t;

// Values that can be placed in the type field of a blocking_call_t struct
#define BLOCKING_CALL_NONE          0
#define BLOCKING_CALL_RTC           1
#define BLOCKING_CALL_PROCESS_EXEC  2
#define BLOCKING_CALL_TERMINAL_READ 3
#define BLOCKING_CALL_UDP_READ      4

typedef struct pcb_t {
	// A dynamic array of the files that are being used by the process
	file_dyn_arr files;
	// A dynamic array of the indices of the 4MB pages allocated to this process (excluding video memory)
	page_mapping_dyn_arr large_page_mappings;
	// The address of the base of the kernel stack
	void *kernel_stack_base;
	// The TTY that this process is in (1-based indices)
	uint8_t tty;
	// The PID of the process; a negative value indicates that this PCB does not represent a valid process
	int32_t pid;
	// The PID of the parent process
	int32_t parent_pid;
	// The buffer of arguments
	int8_t args[TERMINAL_SIZE];
	// The state of the process (either PROCESS_RUNNING, PROCESS_SLEEPING, or PROCESS_STOPPING)
	uint8_t state;
	// If the state is PROCESS_SLEEPING (due to a blocking call), data associated with the blocking call
	blocking_call_t blocking_call;
	// The values of ESP, EIP, and EBP before the scheduler switched to another process
	// All other registers are stored on the stack by the timer linkage
	scheduler_context context;
	// The signal handlers for the process
	signal_handler signal_handlers[NUM_SIGNALS];
	// The status of each signal (options can be found in signals.h)
	int32_t signal_status[NUM_SIGNALS];
	// The data associated with any pending signal
	uint32_t signal_data[NUM_SIGNALS];
} pcb_t;

typedef DYNAMIC_ARRAY(pcb_t, pcb_dyn_arr) pcb_dyn_arr;

// Initializes any supporting data structures for managing user level processes
int init_processes();
// Starts the process associated with the given shell command
int32_t process_execute(const char *command, uint8_t has_parent, uint8_t tty, uint8_t save_context);
// Halts the current process and returns the provided status code to the parent process
int32_t process_halt(uint16_t status);
// Maps video memory for the current userspace program to either video memory or a buffer depending
//  on whether or not the current program is in the active TTY
int32_t process_vidmap(uint8_t **screen_start);
// Marks the provided process as asleep and spins until the current quantum is complete,
//  in the case that the current quantum is the process being put to sleep
int32_t process_sleep(int32_t pid);
int32_t unmap_process(int32_t pid);
int32_t map_process(int32_t pid);
// Wakes up the process of provided PID
int32_t process_wake(int32_t pid);
// Checks if the given region lies within the memory assigned to the process with the given PID
int8_t is_userspace_region_valid(void *ptr, uint32_t size, int32_t pid);
// Checks if the given string lies within the memory assigned to the process with the given PID
int8_t is_userspace_string_valid(void *ptr, int32_t pid);
// Gets the current PID from tss.esp0
int32_t get_pid();
// Gets the current pcb from the stack
pcb_t* get_pcb();
// Gets a pointer to the current userspace program's registers stored on the kernel stack
process_context *get_user_context();
// Gets the pointer to the start of video memory for the given TTY
void *get_vid_mem(uint8_t tty);
// Switches from the current TTY to the provided TTY
int32_t tty_switch(uint8_t tty);
// The handler called by the timer that switches to the next process 
void scheduler_interrupt_handler();

// The currently active TTY
extern uint8_t active_tty;
// A dynamic array of the PCBs, where each index corresponds to a PID
extern pcb_dyn_arr pcbs;

// A spinlock that should be acquired whenever anything that changes based on the TTY is used
//  such as the return value of get_vid_mem, for example
extern struct spinlock_t tty_spin_lock;
// A spinlock that prevents the pcbs dynamic array from being modified concurrently
extern struct spinlock_t pcb_spin_lock;

#endif
