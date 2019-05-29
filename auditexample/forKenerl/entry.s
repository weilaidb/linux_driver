/*
 *  linux/arch/i386/entry.S
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * entry.S contains the system-call and fault low-level handling routines.
 * This also contains the timer-interrupt handler, as well as all interrupts
 * and faults that can result in a task-switch.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call.
 *
 * I changed all the .align's to 4 (16 byte alignment), as that's faster
 * on a 486.
 *
 * Stack layout in 'ret_from_system_call':
 * 	ptrace needs to have all regs on the stack.
 *	if the order here is changed, it needs to be
 *	updated in fork.c:copy_process, signal.c:do_signal,
 *	ptrace.c and ptrace.h
 *
 *	 0(%esp) - %ebx
 *	 4(%esp) - %ecx
 *	 8(%esp) - %edx
 *       C(%esp) - %esi
 *	10(%esp) - %edi
 *	14(%esp) - %ebp
 *	18(%esp) - %eax
 *	1C(%esp) - %ds
 *	20(%esp) - %es
 *	24(%esp) - orig_eax
 *	28(%esp) - %eip
 *	2C(%esp) - %cs
 *	30(%esp) - %eflags
 *	34(%esp) - %oldesp
 *	38(%esp) - %oldss
 *
 * "current" is in register %ebx during any slow entries.
 */

#include <linux/config.h>
#include <linux/sys.h>
#include <linux/linkage.h>
#include <asm/segment.h>
#define ASSEMBLY
#include <asm/smp.h>

EBX		= 0x00
ECX		= 0x04
EDX		= 0x08
ESI		= 0x0C
EDI		= 0x10
EBP		= 0x14
EAX		= 0x18
DS		= 0x1C
ES		= 0x20
ORIG_EAX	= 0x24
EIP		= 0x28
CS		= 0x2C
EFLAGS		= 0x30
OLDESP		= 0x34
OLDSS		= 0x38

CF_MASK		= 0x00000001
IF_MASK		= 0x00000200
NT_MASK		= 0x00004000
VM_MASK		= 0x00020000

/*
 * these are offsets into the task-struct.
 */
state		=  0
flags		=  4
sigpending	=  8
addr_limit	= 12
exec_domain	= 16
need_resched	= 20
tsk_ptrace	= 24
processor	= 52

ENOSYS = 38


#define SAVE_ALL \
	cld; \
	pushl %es; \
	pushl %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx; \
	movl $(__KERNEL_DS),%edx; \
	movl %edx,%ds; \
	movl %edx,%es;

#define RESTORE_ALL	\
	popl %ebx;	\
	popl %ecx;	\
	popl %edx;	\
	popl %esi;	\
	popl %edi;	\
	popl %ebp;	\
	popl %eax;	\
1:	popl %ds;	\
2:	popl %es;	\
	addl $4,%esp;	\
3:	iret;		\
.section .fixup,"ax";	\
4:	movl $0,(%esp);	\
	jmp 1b;		\
5:	movl $0,(%esp);	\
	jmp 2b;		\
6:	pushl %ss;	\
	popl %ds;	\
	pushl %ss;	\
	popl %es;	\
	pushl $11;	\
	call do_exit;	\
.previous;		\
.section __ex_table,"a";\
	.align 4;	\
	.long 1b,4b;	\
	.long 2b,5b;	\
	.long 3b,6b;	\
.previous

#define GET_CURRENT(reg) \
	movl $-8192, reg; \
	andl %esp, reg

ENTRY(lcall7)
	pushfl			# We get a different stack layout with call gates,
	pushl %eax		# which has to be cleaned up later..
	SAVE_ALL
	movl EIP(%esp),%eax	# due to call gates, this is eflags, not eip..
	movl CS(%esp),%edx	# this is eip..
	movl EFLAGS(%esp),%ecx	# and this is cs..
	movl %eax,EFLAGS(%esp)	#
	movl %edx,EIP(%esp)	# Now we move them to their "normal" places
	movl %ecx,CS(%esp)	#
	movl %esp,%ebx
	pushl %ebx
	andl $-8192,%ebx	# GET_CURRENT
	movl exec_domain(%ebx),%edx	# Get the execution domain
	movl 4(%edx),%edx	# Get the lcall7 handler for the domain
	pushl $0x7
	call *%edx
	addl $4, %esp
	popl %eax
	jmp ret_from_sys_call

ENTRY(lcall27)
	pushfl			# We get a different stack layout with call gates,
	pushl %eax		# which has to be cleaned up later..
	SAVE_ALL
	movl EIP(%esp),%eax	# due to call gates, this is eflags, not eip..
	movl CS(%esp),%edx	# this is eip..
	movl EFLAGS(%esp),%ecx	# and this is cs..
	movl %eax,EFLAGS(%esp)	#
	movl %edx,EIP(%esp)	# Now we move them to their "normal" places
	movl %ecx,CS(%esp)	#
	movl %esp,%ebx
	pushl %ebx
	andl $-8192,%ebx	# GET_CURRENT
	movl exec_domain(%ebx),%edx	# Get the execution domain
	movl 4(%edx),%edx	# Get the lcall7 handler for the domain
	pushl $0x27
	call *%edx
	addl $4, %esp
	popl %eax
	jmp ret_from_sys_call


ENTRY(ret_from_fork)
	pushl %ebx
	call SYMBOL_NAME(schedule_tail)
	addl $4, %esp
	GET_CURRENT(%ebx)
	testb $0x02,tsk_ptrace(%ebx)	# PT_TRACESYS
	jne tracesys_exit
	jmp	ret_from_sys_call

#if defined(CONFIG_KDB)
ENTRY(kdb_call)
	pushl %eax              # save orig EAX
	SAVE_ALL
	pushl %esp              # struct pt_regs
	pushl $0                # error_code
	pushl $7                # KDB_REASON_ENTRY
	call  SYMBOL_NAME(kdb)
	addl  $12,%esp          # remove args
	RESTORE_ALL
#endif

/*
 * Return to user mode is not as complex as all this looks,
 * but we want the default path for a system call return to
 * go as quickly as possible which is why some of this is
 * less clear than it otherwise should be.
 */

ENTRY(system_call)
	pushl %eax			# save orig_eax
	SAVE_ALL
	GET_CURRENT(%ebx)
	cmpl $(NR_syscalls),%eax
	jae badsys
	testb $0x02,tsk_ptrace(%ebx)	# PT_TRACESYS
	jne tracesys
	call *SYMBOL_NAME(sys_call_table)(,%eax,4)
	movl %eax,EAX(%esp)		# save the return value
        cmpl $1, ORIG_EAX(%esp)
        je auditsys
        cmpl $2, ORIG_EAX(%esp)
        je auditsys
        cmpl $3, ORIG_EAX(%esp)
        je auditsys
        cmpl $4, ORIG_EAX(%esp)
        je auditsys
        cmpl $5, ORIG_EAX(%esp)
        je auditsys
        cmpl $6, ORIG_EAX(%esp)
        je auditsys
        cmpl $7, ORIG_EAX(%esp)
        je auditsys
        cmpl $8, ORIG_EAX(%esp)
        je auditsys
        cmpl $9, ORIG_EAX(%esp)
        je auditsys
        cmpl $10, ORIG_EAX(%esp)
        je auditsys
        cmpl $11, ORIG_EAX(%esp)
        je auditsys
        cmpl $12, ORIG_EAX(%esp)
        je auditsys
        cmpl $13, ORIG_EAX(%esp)
        je auditsys
        cmpl $14, ORIG_EAX(%esp)
        je auditsys
        cmpl $15, ORIG_EAX(%esp)
        je auditsys
        cmpl $16, ORIG_EAX(%esp)
        je auditsys
        cmpl $18, ORIG_EAX(%esp)
        je auditsys
        cmpl $19, ORIG_EAX(%esp)
        je auditsys
        cmpl $20, ORIG_EAX(%esp)
        je auditsys
        cmpl $21, ORIG_EAX(%esp)
        je auditsys
        cmpl $22, ORIG_EAX(%esp)
        je auditsys
        cmpl $23, ORIG_EAX(%esp)
        je auditsys
        cmpl $24, ORIG_EAX(%esp)
        je auditsys
        cmpl $25, ORIG_EAX(%esp)
        je auditsys
        cmpl $26, ORIG_EAX(%esp)
        je auditsys
        cmpl $27, ORIG_EAX(%esp)
        je auditsys
        cmpl $28, ORIG_EAX(%esp)
        je auditsys
        cmpl $29, ORIG_EAX(%esp)
        je auditsys
        cmpl $30, ORIG_EAX(%esp)
        je auditsys
        cmpl $33, ORIG_EAX(%esp)
        je auditsys
        cmpl $34, ORIG_EAX(%esp)
        je auditsys
        cmpl $36, ORIG_EAX(%esp)
        je auditsys
        cmpl $37, ORIG_EAX(%esp)
        je auditsys
        cmpl $38, ORIG_EAX(%esp)
        je auditsys
        cmpl $39, ORIG_EAX(%esp)
        je auditsys
        cmpl $40, ORIG_EAX(%esp)
        je auditsys
        cmpl $41, ORIG_EAX(%esp)
        je auditsys
        cmpl $42, ORIG_EAX(%esp)
        je auditsys
        cmpl $43, ORIG_EAX(%esp)
        je auditsys
        cmpl $45, ORIG_EAX(%esp)
        je auditsys
        cmpl $46, ORIG_EAX(%esp)
        je auditsys
        cmpl $47, ORIG_EAX(%esp)
        je auditsys
        cmpl $48, ORIG_EAX(%esp)
        je auditsys
        cmpl $49, ORIG_EAX(%esp)
        je auditsys
        cmpl $50, ORIG_EAX(%esp)
        je auditsys
        cmpl $51, ORIG_EAX(%esp)
        je auditsys
        cmpl $52, ORIG_EAX(%esp)
        je auditsys
        cmpl $54, ORIG_EAX(%esp)
        je auditsys
        cmpl $55, ORIG_EAX(%esp)
        je auditsys
        cmpl $57, ORIG_EAX(%esp)
        je auditsys
        cmpl $59, ORIG_EAX(%esp)
        je auditsys
        cmpl $60, ORIG_EAX(%esp)
        je auditsys
        cmpl $61, ORIG_EAX(%esp)
        je auditsys
        cmpl $62, ORIG_EAX(%esp)
        je auditsys
        cmpl $63, ORIG_EAX(%esp)
        je auditsys
        cmpl $64, ORIG_EAX(%esp)
        je auditsys
        cmpl $65, ORIG_EAX(%esp)
        je auditsys
        cmpl $66, ORIG_EAX(%esp)
        je auditsys
        cmpl $67, ORIG_EAX(%esp)
        je auditsys
        cmpl $68, ORIG_EAX(%esp)
        je auditsys
        cmpl $69, ORIG_EAX(%esp)
        je auditsys
        cmpl $70, ORIG_EAX(%esp)
        je auditsys
        cmpl $71, ORIG_EAX(%esp)
        je auditsys
        cmpl $72, ORIG_EAX(%esp)
        je auditsys
        cmpl $73, ORIG_EAX(%esp)
        je auditsys
        cmpl $74, ORIG_EAX(%esp)
        je auditsys
        cmpl $75, ORIG_EAX(%esp)
        je auditsys
        cmpl $76, ORIG_EAX(%esp)
        je auditsys
        cmpl $77, ORIG_EAX(%esp)
        je auditsys
        cmpl $78, ORIG_EAX(%esp)
        je auditsys
        cmpl $79, ORIG_EAX(%esp)
        je auditsys
        cmpl $80, ORIG_EAX(%esp)
        je auditsys
        cmpl $81, ORIG_EAX(%esp)
        je auditsys
        cmpl $82, ORIG_EAX(%esp)
        je auditsys
        cmpl $83, ORIG_EAX(%esp)
        je auditsys
        cmpl $84, ORIG_EAX(%esp)
        je auditsys
        cmpl $85, ORIG_EAX(%esp)
        je auditsys
        cmpl $86, ORIG_EAX(%esp)
        je auditsys
        cmpl $87, ORIG_EAX(%esp)
        je auditsys
        cmpl $88, ORIG_EAX(%esp)
        je auditsys
        cmpl $89, ORIG_EAX(%esp)
        je auditsys
        cmpl $90, ORIG_EAX(%esp)
        je auditsys
        cmpl $91, ORIG_EAX(%esp)
        je auditsys
        cmpl $92, ORIG_EAX(%esp)
        je auditsys
        cmpl $93, ORIG_EAX(%esp)
        je auditsys
        cmpl $94, ORIG_EAX(%esp)
        je auditsys
        cmpl $95, ORIG_EAX(%esp)
        je auditsys
        cmpl $96, ORIG_EAX(%esp)
        je auditsys
        cmpl $97, ORIG_EAX(%esp)
        je auditsys
        cmpl $99, ORIG_EAX(%esp)
        je auditsys
        cmpl $100, ORIG_EAX(%esp)
        je auditsys
        cmpl $101, ORIG_EAX(%esp)
        je auditsys
        cmpl $102, ORIG_EAX(%esp)
        je auditsys
        cmpl $103, ORIG_EAX(%esp)
        je auditsys
        cmpl $104, ORIG_EAX(%esp)
        je auditsys
        cmpl $105, ORIG_EAX(%esp)
        je auditsys
        cmpl $106, ORIG_EAX(%esp)
        je auditsys
        cmpl $107, ORIG_EAX(%esp)
        je auditsys
        cmpl $108, ORIG_EAX(%esp)
        je auditsys
        cmpl $109, ORIG_EAX(%esp)
        je auditsys
        cmpl $110, ORIG_EAX(%esp)
        je auditsys
        cmpl $111, ORIG_EAX(%esp)
        je auditsys
        cmpl $113, ORIG_EAX(%esp)
        je auditsys
        cmpl $114, ORIG_EAX(%esp)
        je auditsys
        cmpl $115, ORIG_EAX(%esp)
        je auditsys
        cmpl $116, ORIG_EAX(%esp)
        je auditsys
        cmpl $117, ORIG_EAX(%esp)
        je auditsys
        cmpl $118, ORIG_EAX(%esp)
        je auditsys
        cmpl $119, ORIG_EAX(%esp)
        je auditsys
        cmpl $120, ORIG_EAX(%esp)
        je auditsys
        cmpl $121, ORIG_EAX(%esp)
        je auditsys
        cmpl $122, ORIG_EAX(%esp)
        je auditsys
        cmpl $123, ORIG_EAX(%esp)
        je auditsys
        cmpl $124, ORIG_EAX(%esp)
        je auditsys
        cmpl $125, ORIG_EAX(%esp)
        je auditsys
        cmpl $126, ORIG_EAX(%esp)
        je auditsys
        cmpl $127, ORIG_EAX(%esp)
        je auditsys
        cmpl $128, ORIG_EAX(%esp)
        je auditsys
        cmpl $129, ORIG_EAX(%esp)
        je auditsys
        cmpl $130, ORIG_EAX(%esp)
        je auditsys
        cmpl $131, ORIG_EAX(%esp)
        je auditsys
        cmpl $132, ORIG_EAX(%esp)
        je auditsys
        cmpl $133, ORIG_EAX(%esp)
        je auditsys
        cmpl $134, ORIG_EAX(%esp)
        je auditsys
        cmpl $135, ORIG_EAX(%esp)
        je auditsys
        cmpl $136, ORIG_EAX(%esp)
        je auditsys
        cmpl $138, ORIG_EAX(%esp)
        je auditsys
        cmpl $139, ORIG_EAX(%esp)
        je auditsys
        cmpl $140, ORIG_EAX(%esp)
        je auditsys
        cmpl $141, ORIG_EAX(%esp)
        je auditsys
        cmpl $142, ORIG_EAX(%esp)
        je auditsys
        cmpl $143, ORIG_EAX(%esp)
        je auditsys
        cmpl $144, ORIG_EAX(%esp)
        je auditsys
        cmpl $145, ORIG_EAX(%esp)
        je auditsys
        cmpl $146, ORIG_EAX(%esp)
        je auditsys
        cmpl $147, ORIG_EAX(%esp)
        je auditsys
        cmpl $148, ORIG_EAX(%esp)
        je auditsys
        cmpl $149, ORIG_EAX(%esp)
        je auditsys
        cmpl $150, ORIG_EAX(%esp)
        je auditsys
        cmpl $151, ORIG_EAX(%esp)
        je auditsys
        cmpl $152, ORIG_EAX(%esp)
        je auditsys
        cmpl $153, ORIG_EAX(%esp)
        je auditsys
        cmpl $154, ORIG_EAX(%esp)
        je auditsys
        cmpl $155, ORIG_EAX(%esp)
        je auditsys
        cmpl $156, ORIG_EAX(%esp)
        je auditsys
        cmpl $157, ORIG_EAX(%esp)
        je auditsys
        cmpl $158, ORIG_EAX(%esp)
        je auditsys
        cmpl $159, ORIG_EAX(%esp)
        je auditsys
        cmpl $160, ORIG_EAX(%esp)
        je auditsys
        cmpl $161, ORIG_EAX(%esp)
        je auditsys
        cmpl $162, ORIG_EAX(%esp)
        je auditsys
        cmpl $163, ORIG_EAX(%esp)
        je auditsys
        cmpl $164, ORIG_EAX(%esp)
        je auditsys
        cmpl $165, ORIG_EAX(%esp)
        je auditsys
        cmpl $166, ORIG_EAX(%esp)
        je auditsys
        cmpl $167, ORIG_EAX(%esp)
        je auditsys
        cmpl $168, ORIG_EAX(%esp)
        je auditsys
        cmpl $169, ORIG_EAX(%esp)
        je auditsys
        cmpl $170, ORIG_EAX(%esp)
        je auditsys
        cmpl $171, ORIG_EAX(%esp)
        je auditsys
        cmpl $172, ORIG_EAX(%esp)
        je auditsys
        cmpl $173, ORIG_EAX(%esp)
        je auditsys
        cmpl $174, ORIG_EAX(%esp)
        je auditsys
        cmpl $175, ORIG_EAX(%esp)
        je auditsys
        cmpl $176, ORIG_EAX(%esp)
        je auditsys
        cmpl $177, ORIG_EAX(%esp)
        je auditsys
        cmpl $178, ORIG_EAX(%esp)
        je auditsys
        cmpl $179, ORIG_EAX(%esp)
        je auditsys
        cmpl $180, ORIG_EAX(%esp)
        je auditsys
        cmpl $181, ORIG_EAX(%esp)
        je auditsys
        cmpl $182, ORIG_EAX(%esp)
        je auditsys
        cmpl $183, ORIG_EAX(%esp)
        je auditsys
        cmpl $184, ORIG_EAX(%esp)
        je auditsys
        cmpl $185, ORIG_EAX(%esp)
        je auditsys
        cmpl $186, ORIG_EAX(%esp)
        je auditsys
        cmpl $187, ORIG_EAX(%esp)
        je auditsys
        cmpl $190, ORIG_EAX(%esp)
        je auditsys
        cmpl $191, ORIG_EAX(%esp)
        je auditsys
        cmpl $192, ORIG_EAX(%esp)
        je auditsys
        cmpl $193, ORIG_EAX(%esp)
        je auditsys
        cmpl $194, ORIG_EAX(%esp)
        je auditsys
        cmpl $195, ORIG_EAX(%esp)
        je auditsys
        cmpl $196, ORIG_EAX(%esp)
        je auditsys
        cmpl $197, ORIG_EAX(%esp)
        je auditsys
        cmpl $198, ORIG_EAX(%esp)
        je auditsys
        cmpl $199, ORIG_EAX(%esp)
        je auditsys
        cmpl $200, ORIG_EAX(%esp)
        je auditsys
        cmpl $201, ORIG_EAX(%esp)
        je auditsys
        cmpl $202, ORIG_EAX(%esp)
        je auditsys
        cmpl $203, ORIG_EAX(%esp)
        je auditsys
        cmpl $204, ORIG_EAX(%esp)
        je auditsys
        cmpl $205, ORIG_EAX(%esp)
        je auditsys
        cmpl $206, ORIG_EAX(%esp)
        je auditsys
        cmpl $207, ORIG_EAX(%esp)
        je auditsys
        cmpl $208, ORIG_EAX(%esp)
        je auditsys
        cmpl $209, ORIG_EAX(%esp)
        je auditsys
        cmpl $210, ORIG_EAX(%esp)
        je auditsys
        cmpl $211, ORIG_EAX(%esp)
        je auditsys
        cmpl $212, ORIG_EAX(%esp)
        je auditsys
        cmpl $213, ORIG_EAX(%esp)
        je auditsys
        cmpl $214, ORIG_EAX(%esp)
        je auditsys
        cmpl $215, ORIG_EAX(%esp)
        je auditsys
        cmpl $216, ORIG_EAX(%esp)
        je auditsys
        cmpl $217, ORIG_EAX(%esp)
        je auditsys
        cmpl $218, ORIG_EAX(%esp)
        je auditsys
        cmpl $219, ORIG_EAX(%esp)
        je auditsys
        cmpl $220, ORIG_EAX(%esp)
        je auditsys
        cmpl $221, ORIG_EAX(%esp)
        je auditsys
        cmpl $224, ORIG_EAX(%esp)
        je auditsys
        cmpl $225, ORIG_EAX(%esp)
        je auditsys

ENTRY(ret_from_sys_call)
	cli				# need_resched and signals atomic test
	cmpl $0,need_resched(%ebx)
	jne reschedule
	cmpl $0,sigpending(%ebx)
	jne signal_return
restore_all:
	RESTORE_ALL

	ALIGN
signal_return:
	sti				# we can get here from an interrupt handler
	testl $(VM_MASK),EFLAGS(%esp)
	movl %esp,%eax
	jne v86_signal_return
	xorl %edx,%edx
	call SYMBOL_NAME(do_signal)
	jmp restore_all

	ALIGN
v86_signal_return:
	call SYMBOL_NAME(save_v86_state)
	movl %eax,%esp
	xorl %edx,%edx
	call SYMBOL_NAME(do_signal)
	jmp restore_all

	ALIGN
auditsys:
	pushl %eax		# pass in return status
	pushl 0x28(%esp)	# pass in syscall number
	call SYMBOL_NAME(syscall_audit);
	popl %eax		# remove orig_eax from stack
	popl %eax		# remove eax from stack
	jmp ret_from_sys_call
tracesys:
	movl $-ENOSYS,EAX(%esp)
	call SYMBOL_NAME(syscall_trace)
	movl ORIG_EAX(%esp),%eax
	cmpl $(NR_syscalls),%eax
	jae tracesys_exit
	call *SYMBOL_NAME(sys_call_table)(,%eax,4)
	movl %eax,EAX(%esp)		# save the return value
tracesys_exit:
	call SYMBOL_NAME(syscall_trace)
	jmp ret_from_sys_call
badsys:
	movl $-ENOSYS,EAX(%esp)
	jmp ret_from_sys_call

	ALIGN
ENTRY(ret_from_intr)
	GET_CURRENT(%ebx)
ret_from_exception:
	movl EFLAGS(%esp),%eax		# mix EFLAGS and CS
	movb CS(%esp),%al
	testl $(VM_MASK | 3),%eax	# return to VM86 mode or non-supervisor?
	jne ret_from_sys_call
	jmp restore_all

	ALIGN
reschedule:
	call SYMBOL_NAME(schedule)    # test
	jmp ret_from_sys_call

ENTRY(divide_error)
	pushl $0		# no error code
	pushl $ SYMBOL_NAME(do_divide_error)
	ALIGN
error_code:
	pushl %ds
	pushl %eax
	xorl %eax,%eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	decl %eax			# eax = -1
	pushl %ecx
	pushl %ebx
	cld
	movl %es,%ecx
	movl ORIG_EAX(%esp), %esi	# get the error code
	movl ES(%esp), %edi		# get the function address
	movl %eax, ORIG_EAX(%esp)
	movl %ecx, ES(%esp)
	movl %esp,%edx
	pushl %esi			# push the error code
	pushl %edx			# push the pt_regs pointer
	movl $(__KERNEL_DS),%edx
	movl %edx,%ds
	movl %edx,%es
	GET_CURRENT(%ebx)
	call *%edi
	addl $8,%esp
	jmp ret_from_exception

ENTRY(coprocessor_error)
	pushl $0
	pushl $ SYMBOL_NAME(do_coprocessor_error)
	jmp error_code

ENTRY(simd_coprocessor_error)
	pushl $0
	pushl $ SYMBOL_NAME(do_simd_coprocessor_error)
	jmp error_code

ENTRY(device_not_available)
	pushl $-1		# mark this as an int
	SAVE_ALL
	GET_CURRENT(%ebx)
	movl %cr0,%eax
	testl $0x4,%eax			# EM (math emulation bit)
	jne device_not_available_emulate
	call SYMBOL_NAME(math_state_restore)
	jmp ret_from_exception
device_not_available_emulate:
	pushl $0		# temporary storage for ORIG_EIP
	call  SYMBOL_NAME(math_emulate)
	addl $4,%esp
	jmp ret_from_exception

ENTRY(debug)
	pushl $0
	pushl $ SYMBOL_NAME(do_debug)
	jmp error_code

ENTRY(nmi)
	pushl %eax
	SAVE_ALL
	movl %esp,%edx
	pushl $0
	pushl %edx
	call SYMBOL_NAME(do_nmi)
	addl $8,%esp
	RESTORE_ALL

ENTRY(int3)
	pushl $0
	pushl $ SYMBOL_NAME(do_int3)
	jmp error_code

ENTRY(overflow)
	pushl $0
	pushl $ SYMBOL_NAME(do_overflow)
	jmp error_code

ENTRY(bounds)
	pushl $0
	pushl $ SYMBOL_NAME(do_bounds)
	jmp error_code

ENTRY(invalid_op)
	pushl $0
	pushl $ SYMBOL_NAME(do_invalid_op)
	jmp error_code

ENTRY(coprocessor_segment_overrun)
	pushl $0
	pushl $ SYMBOL_NAME(do_coprocessor_segment_overrun)
	jmp error_code

ENTRY(double_fault)
	pushl $ SYMBOL_NAME(do_double_fault)
	jmp error_code

ENTRY(invalid_TSS)
	pushl $ SYMBOL_NAME(do_invalid_TSS)
	jmp error_code

ENTRY(segment_not_present)
	pushl $ SYMBOL_NAME(do_segment_not_present)
	jmp error_code

ENTRY(stack_segment)
	pushl $ SYMBOL_NAME(do_stack_segment)
	jmp error_code

ENTRY(general_protection)
	pushl $ SYMBOL_NAME(do_general_protection)
	jmp error_code

ENTRY(alignment_check)
	pushl $ SYMBOL_NAME(do_alignment_check)
	jmp error_code

#if defined(CONFIG_KDB)
ENTRY(page_fault_mca)
	pushl %ecx
	pushl %edx
	pushl %eax
	movl  $473,%ecx
	rdmsr
	andl  $0xfffffffe,%eax		/* Disable last branch recording */
	wrmsr
	popl  %eax
	popl  %edx
	popl  %ecx
	pushl $ SYMBOL_NAME(do_page_fault)
	jmp error_code
#endif

ENTRY(page_fault)
	pushl $ SYMBOL_NAME(do_page_fault)
	jmp error_code

ENTRY(machine_check)
	pushl $0
	pushl $ SYMBOL_NAME(do_machine_check)
	jmp error_code

ENTRY(spurious_interrupt_bug)
	pushl $0
	pushl $ SYMBOL_NAME(do_spurious_interrupt_bug)
	jmp error_code

.data
ENTRY(sys_call_table)
	.long SYMBOL_NAME(sys_ni_syscall)	/* 0  -  old "setup()" system call*/
	.long SYMBOL_NAME(sys_exit)
	.long SYMBOL_NAME(sys_fork)
	.long SYMBOL_NAME(sys_read)
	.long SYMBOL_NAME(sys_write)
	.long SYMBOL_NAME(sys_open)		/* 5 */
	.long SYMBOL_NAME(sys_close)
	.long SYMBOL_NAME(sys_waitpid)
	.long SYMBOL_NAME(sys_creat)
	.long SYMBOL_NAME(sys_link)
	.long SYMBOL_NAME(sys_unlink)		/* 10 */
	.long SYMBOL_NAME(sys_execve)
	.long SYMBOL_NAME(sys_chdir)
	.long SYMBOL_NAME(sys_time)
	.long SYMBOL_NAME(sys_mknod)
	.long SYMBOL_NAME(sys_chmod)		/* 15 */
	.long SYMBOL_NAME(sys_lchown16)
	.long SYMBOL_NAME(sys_ni_syscall)				/* old break syscall holder */
	.long SYMBOL_NAME(sys_stat)
	.long SYMBOL_NAME(sys_lseek)
	.long SYMBOL_NAME(sys_getpid)		/* 20 */
	.long SYMBOL_NAME(sys_mount)
	.long SYMBOL_NAME(sys_oldumount)
	.long SYMBOL_NAME(sys_setuid16)
	.long SYMBOL_NAME(sys_getuid16)
	.long SYMBOL_NAME(sys_stime)		/* 25 */
	.long SYMBOL_NAME(sys_ptrace)
	.long SYMBOL_NAME(sys_alarm)
	.long SYMBOL_NAME(sys_fstat)
	.long SYMBOL_NAME(sys_pause)
	.long SYMBOL_NAME(sys_utime)		/* 30 */
	.long SYMBOL_NAME(sys_ni_syscall)				/* old stty syscall holder */
	.long SYMBOL_NAME(sys_ni_syscall)				/* old gtty syscall holder */
	.long SYMBOL_NAME(sys_access)
	.long SYMBOL_NAME(sys_nice)
	.long SYMBOL_NAME(sys_ni_syscall)	/* 35 */		/* old ftime syscall holder */
	.long SYMBOL_NAME(sys_sync)
	.long SYMBOL_NAME(sys_kill)
	.long SYMBOL_NAME(sys_rename)
	.long SYMBOL_NAME(sys_mkdir)
	.long SYMBOL_NAME(sys_rmdir)		/* 40 */
	.long SYMBOL_NAME(sys_dup)
	.long SYMBOL_NAME(sys_pipe)
	.long SYMBOL_NAME(sys_times)
	.long SYMBOL_NAME(sys_ni_syscall)				/* old prof syscall holder */
	.long SYMBOL_NAME(sys_brk)		/* 45 */
	.long SYMBOL_NAME(sys_setgid16)
	.long SYMBOL_NAME(sys_getgid16)
	.long SYMBOL_NAME(sys_signal)
	.long SYMBOL_NAME(sys_geteuid16)
	.long SYMBOL_NAME(sys_getegid16)	/* 50 */
	.long SYMBOL_NAME(sys_acct)
	.long SYMBOL_NAME(sys_umount)					/* recycled never used phys() */
	.long SYMBOL_NAME(sys_ni_syscall)				/* old lock syscall holder */
	.long SYMBOL_NAME(sys_ioctl)
	.long SYMBOL_NAME(sys_fcntl)		/* 55 */
	.long SYMBOL_NAME(sys_ni_syscall)				/* old mpx syscall holder */
	.long SYMBOL_NAME(sys_setpgid)
	.long SYMBOL_NAME(sys_ni_syscall)				/* old ulimit syscall holder */
	.long SYMBOL_NAME(sys_olduname)
	.long SYMBOL_NAME(sys_umask)		/* 60 */
	.long SYMBOL_NAME(sys_chroot)
	.long SYMBOL_NAME(sys_ustat)
	.long SYMBOL_NAME(sys_dup2)
	.long SYMBOL_NAME(sys_getppid)
	.long SYMBOL_NAME(sys_getpgrp)		/* 65 */
	.long SYMBOL_NAME(sys_setsid)
	.long SYMBOL_NAME(sys_sigaction)
	.long SYMBOL_NAME(sys_sgetmask)
	.long SYMBOL_NAME(sys_ssetmask)
	.long SYMBOL_NAME(sys_setreuid16)	/* 70 */
	.long SYMBOL_NAME(sys_setregid16)
	.long SYMBOL_NAME(sys_sigsuspend)
	.long SYMBOL_NAME(sys_sigpending)
	.long SYMBOL_NAME(sys_sethostname)
	.long SYMBOL_NAME(sys_setrlimit)	/* 75 */
	.long SYMBOL_NAME(sys_old_getrlimit)
	.long SYMBOL_NAME(sys_getrusage)
	.long SYMBOL_NAME(sys_gettimeofday)
	.long SYMBOL_NAME(sys_settimeofday)
	.long SYMBOL_NAME(sys_getgroups16)	/* 80 */
	.long SYMBOL_NAME(sys_setgroups16)
	.long SYMBOL_NAME(old_select)
	.long SYMBOL_NAME(sys_symlink)
	.long SYMBOL_NAME(sys_lstat)
	.long SYMBOL_NAME(sys_readlink)		/* 85 */
	.long SYMBOL_NAME(sys_uselib)
	.long SYMBOL_NAME(sys_swapon)
	.long SYMBOL_NAME(sys_reboot)
	.long SYMBOL_NAME(old_readdir)
	.long SYMBOL_NAME(old_mmap)		/* 90 */
	.long SYMBOL_NAME(sys_munmap)
	.long SYMBOL_NAME(sys_truncate)
	.long SYMBOL_NAME(sys_ftruncate)
	.long SYMBOL_NAME(sys_fchmod)
	.long SYMBOL_NAME(sys_fchown16)		/* 95 */
	.long SYMBOL_NAME(sys_getpriority)
	.long SYMBOL_NAME(sys_setpriority)
	.long SYMBOL_NAME(sys_ni_syscall)				/* old profil syscall holder */
	.long SYMBOL_NAME(sys_statfs)
	.long SYMBOL_NAME(sys_fstatfs)		/* 100 */
	.long SYMBOL_NAME(sys_ioperm)
	.long SYMBOL_NAME(sys_socketcall)
	.long SYMBOL_NAME(sys_syslog)
	.long SYMBOL_NAME(sys_setitimer)
	.long SYMBOL_NAME(sys_getitimer)	/* 105 */
	.long SYMBOL_NAME(sys_newstat)
	.long SYMBOL_NAME(sys_newlstat)
	.long SYMBOL_NAME(sys_newfstat)
	.long SYMBOL_NAME(sys_uname)
	.long SYMBOL_NAME(sys_iopl)		/* 110 */
	.long SYMBOL_NAME(sys_vhangup)
	.long SYMBOL_NAME(sys_ni_syscall)	/* old "idle" system call */
	.long SYMBOL_NAME(sys_vm86old)
	.long SYMBOL_NAME(sys_wait4)
	.long SYMBOL_NAME(sys_swapoff)		/* 115 */
	.long SYMBOL_NAME(sys_sysinfo)
	.long SYMBOL_NAME(sys_ipc)
	.long SYMBOL_NAME(sys_fsync)
	.long SYMBOL_NAME(sys_sigreturn)
	.long SYMBOL_NAME(sys_clone)		/* 120 */
	.long SYMBOL_NAME(sys_setdomainname)
	.long SYMBOL_NAME(sys_newuname)
	.long SYMBOL_NAME(sys_modify_ldt)
	.long SYMBOL_NAME(sys_adjtimex)
	.long SYMBOL_NAME(sys_mprotect)		/* 125 */
	.long SYMBOL_NAME(sys_sigprocmask)
	.long SYMBOL_NAME(sys_create_module)
	.long SYMBOL_NAME(sys_init_module)
	.long SYMBOL_NAME(sys_delete_module)
	.long SYMBOL_NAME(sys_get_kernel_syms)	/* 130 */
	.long SYMBOL_NAME(sys_quotactl)
	.long SYMBOL_NAME(sys_getpgid)
	.long SYMBOL_NAME(sys_fchdir)
	.long SYMBOL_NAME(sys_bdflush)
	.long SYMBOL_NAME(sys_sysfs)		/* 135 */
	.long SYMBOL_NAME(sys_personality)
	.long SYMBOL_NAME(sys_ni_syscall)	/* for afs_syscall */
	.long SYMBOL_NAME(sys_setfsuid16)
	.long SYMBOL_NAME(sys_setfsgid16)
	.long SYMBOL_NAME(sys_llseek)		/* 140 */
	.long SYMBOL_NAME(sys_getdents)
	.long SYMBOL_NAME(sys_select)
	.long SYMBOL_NAME(sys_flock)
	.long SYMBOL_NAME(sys_msync)
	.long SYMBOL_NAME(sys_readv)		/* 145 */
	.long SYMBOL_NAME(sys_writev)
	.long SYMBOL_NAME(sys_getsid)
	.long SYMBOL_NAME(sys_fdatasync)
	.long SYMBOL_NAME(sys_sysctl)
	.long SYMBOL_NAME(sys_mlock)		/* 150 */
	.long SYMBOL_NAME(sys_munlock)
	.long SYMBOL_NAME(sys_mlockall)
	.long SYMBOL_NAME(sys_munlockall)
	.long SYMBOL_NAME(sys_sched_setparam)
	.long SYMBOL_NAME(sys_sched_getparam)   /* 155 */
	.long SYMBOL_NAME(sys_sched_setscheduler)
	.long SYMBOL_NAME(sys_sched_getscheduler)
	.long SYMBOL_NAME(sys_sched_yield)
	.long SYMBOL_NAME(sys_sched_get_priority_max)
	.long SYMBOL_NAME(sys_sched_get_priority_min)  /* 160 */
	.long SYMBOL_NAME(sys_sched_rr_get_interval)
	.long SYMBOL_NAME(sys_nanosleep)
	.long SYMBOL_NAME(sys_mremap)
	.long SYMBOL_NAME(sys_setresuid16)
	.long SYMBOL_NAME(sys_getresuid16)	/* 165 */
	.long SYMBOL_NAME(sys_vm86)
	.long SYMBOL_NAME(sys_query_module)
	.long SYMBOL_NAME(sys_poll)
	.long SYMBOL_NAME(sys_nfsservctl)
	.long SYMBOL_NAME(sys_setresgid16)	/* 170 */
	.long SYMBOL_NAME(sys_getresgid16)
	.long SYMBOL_NAME(sys_prctl)
	.long SYMBOL_NAME(sys_rt_sigreturn)
	.long SYMBOL_NAME(sys_rt_sigaction)
	.long SYMBOL_NAME(sys_rt_sigprocmask)	/* 175 */
	.long SYMBOL_NAME(sys_rt_sigpending)
	.long SYMBOL_NAME(sys_rt_sigtimedwait)
	.long SYMBOL_NAME(sys_rt_sigqueueinfo)
	.long SYMBOL_NAME(sys_rt_sigsuspend)
	.long SYMBOL_NAME(sys_pread)		/* 180 */
	.long SYMBOL_NAME(sys_pwrite)
	.long SYMBOL_NAME(sys_chown16)
	.long SYMBOL_NAME(sys_getcwd)
	.long SYMBOL_NAME(sys_capget)
	.long SYMBOL_NAME(sys_capset)           /* 185 */
	.long SYMBOL_NAME(sys_sigaltstack)
	.long SYMBOL_NAME(sys_sendfile)
	.long SYMBOL_NAME(sys_ni_syscall)		/* streams1 */
	.long SYMBOL_NAME(sys_ni_syscall)		/* streams2 */
	.long SYMBOL_NAME(sys_vfork)            /* 190 */
	.long SYMBOL_NAME(sys_getrlimit)
	.long SYMBOL_NAME(sys_mmap2)
	.long SYMBOL_NAME(sys_truncate64)
	.long SYMBOL_NAME(sys_ftruncate64)
	.long SYMBOL_NAME(sys_stat64)		/* 195 */
	.long SYMBOL_NAME(sys_lstat64)
	.long SYMBOL_NAME(sys_fstat64)
	.long SYMBOL_NAME(sys_lchown)
	.long SYMBOL_NAME(sys_getuid)
	.long SYMBOL_NAME(sys_getgid)		/* 200 */
	.long SYMBOL_NAME(sys_geteuid)
	.long SYMBOL_NAME(sys_getegid)
	.long SYMBOL_NAME(sys_setreuid)
	.long SYMBOL_NAME(sys_setregid)
	.long SYMBOL_NAME(sys_getgroups)	/* 205 */
	.long SYMBOL_NAME(sys_setgroups)
	.long SYMBOL_NAME(sys_fchown)
	.long SYMBOL_NAME(sys_setresuid)
	.long SYMBOL_NAME(sys_getresuid)
	.long SYMBOL_NAME(sys_setresgid)	/* 210 */
	.long SYMBOL_NAME(sys_getresgid)
	.long SYMBOL_NAME(sys_chown)
	.long SYMBOL_NAME(sys_setuid)
	.long SYMBOL_NAME(sys_setgid)
	.long SYMBOL_NAME(sys_setfsuid)		/* 215 */
	.long SYMBOL_NAME(sys_setfsgid)
	.long SYMBOL_NAME(sys_pivot_root)
	.long SYMBOL_NAME(sys_mincore)
	.long SYMBOL_NAME(sys_madvise)
	.long SYMBOL_NAME(sys_getdents64)	/* 220 */
	.long SYMBOL_NAME(sys_fcntl64)
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for TUX */
	.long SYMBOL_NAME(sys_ni_syscall)	/* Reserved for Security */
	.long SYMBOL_NAME(sys_gettid)
	.long SYMBOL_NAME(sys_readahead)	/* 225 */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for setxattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for lsetxattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for fsetxattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for getxattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* 230 reserved for lgetxattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for fgetxattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for listxattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for llistxattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for flistxattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* 235 reserved for removexattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for lremovexattr */
	.long SYMBOL_NAME(sys_ni_syscall)	/* reserved for fremovexattr */
	.long SYMBOL_NAME(sys_audit)

	.rept NR_syscalls-(.-sys_call_table)/4
		.long SYMBOL_NAME(sys_ni_syscall)
	.endr
