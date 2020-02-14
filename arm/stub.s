# stub.s
# We enter from (eticket) context, in THUMB mode at 0x00010001.

.thumb
.thumb_func
__start:
	add r1, pc, #0x0c
	bx r1
.word 0, 0, 0
.arm
__arm_start:
	# Fix the two words we clobbered on the stack
	ldr r1, =0x1c0
	str r1, [sp]
	ldr r1, =0x20100869
	str r1, [sp, #-4]

	# Make sure we are UID 0
	mov r0, #1
	mov r1, #0
	bl __syscall_set_uid

	# Invalidate the armboot.bin
	ldr r0, =0x00040000
	ldr r1, =0x80000
	bl __syscall_ios_invalidatedcache

	# And go!
	ldr r0, =0x00040000
	ldr r1, =0xdeadbeef
	bl __syscall_boot_new_ios_kernel

restore_state:
	# This shouldn't happen but if it does try out best to recover

	# Return -1337 to PPC-land
	ldr r0, =0xfffffac7

	# Return to the original saved LR that we clobbered
	ldr r3, =0x20100869
	mov lr, r3
	bx lr

# Syscall table
__syscall_set_uid:
	.word 0xe6000570
	bx lr
__syscall_ios_invalidatedcache:
	.word 0xe60007f0
	bx lr
__syscall_boot_new_ios_kernel:
	.word 0xe6000870
	bx lr
