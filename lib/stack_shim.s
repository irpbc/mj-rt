	.text
	.globl _mjrt_alloc
	.extern _mjrt_alloc_impl
_mjrt_alloc:
    # move stack pointer to %rsi which,
    # by calling convention holds the second parameter
	mov %rsp, %rsi
	jmp _mjrt_alloc_impl