	.text
	.globl _mjrt_alloc
	.globl _mjrt_alloc_array
	.extern _mjrt_alloc_impl
	.extern _mjrt_alloc_array_impl
_mjrt_alloc:
    # move stack pointer to %rsi which,
    # by calling convention holds the second parameter
	mov %rsp, %rsi
	jmp _mjrt_alloc_impl
_mjrt_alloc_array:
    # move stack pointer to %rdx which,
    # by calling convention holds the third parameter
	mov %rsp, %rdx
	jmp _mjrt_alloc_array_impl
