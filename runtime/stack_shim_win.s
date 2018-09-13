EXTERN mjrt_alloc_impl : PROC
EXTERN mjrt_alloc_array_impl : PROC
.CODE
mjrt_alloc PROC
; move stack pointer to rdx which,
; by calling convention holds the second parameter
mov rdx, rsp
jmp mjrt_alloc_impl
mjrt_alloc ENDP
mjrt_alloc_array PROC
; move stack pointer to r8 which,
; by calling convention holds the third parameter
mov r8, rsp
jmp mjrt_alloc_array_impl
mjrt_alloc_array ENDP
END