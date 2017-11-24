	irmovq $0x1000, %rsp		# sp
	irmovq $0xcafe, %rsi		# hold constant 1
	call fun1
	printreg %rsi		# should be 0xbeef
	call fun2
	printreg %rax		# should be 0xbeef
	halt
fun1:	irmovq $0xbeef, %rsi
	ret
fun2:	pushq %rsi
	popq %rax
	ret
