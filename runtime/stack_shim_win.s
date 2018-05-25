	.text
	.globl mjrt_alloc
	.globl mjrt_alloc_array
	.extern mjrt_alloc_impl
	.extern mjrt_alloc_array_impl
mjrt_alloc:
    # move stack pointer to %rsi which,
    # by calling convention holds the second parameter
	mov %rsp, %rdx
	jmp mjrt_alloc_impl
mjrt_alloc_array:
    # move stack pointer to %rdx which,
    # by calling convention holds the third parameter
	mov %rsp, %r8
	jmp mjrt_alloc_array_impl
