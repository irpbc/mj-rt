EXTERN mjrt_alloc_impl : PROC
.CODE
mjrt_alloc PROC
; move stack pointer to rdx which,
; by calling convention holds the second parameter
mov rdx, rsp
jmp mjrt_alloc_impl
mjrt_alloc ENDP
END