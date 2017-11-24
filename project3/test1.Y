len:
	irmovq $0xcafe, %rsi		# hold constant 1
	printreg %rsi
	printmem A(%r14)
	mrmovq A(%r14), %rdx     # val = *A
	printreg %rdx
	mrmovq B(%r14), %rcx     # val = *B
	printreg %rcx
	addq %rcx, %rcx         # Test B = 0
	printreg %rcx
	andq %r8,%r8
	subq %rsi,%rcx		# decrement B
	addq %rsi,%rdx		# increment A
	printreg %rcx
	rmmovq %rdx,A(%r14)	# save updated A
	printall
	printmem A(%r14)
	halt
A:	0x88aa
B:	0x1234

