len:
	irmovq $1, %rsi		# hold constant 1
	printmem A(%r14)
	mrmovq A(%r14), %rdx     # val = *A
	printreg %rdx
	mrmovq B(%r14), %rcx     # val = *B
	printreg %rcx
	andq %rcx, %rcx         # Test B = 0
	je Done                 # If zero, goto Done
Loop:
	subq %rsi,%rcx		# decrement B
	addq %rsi,%rdx		# increment A
	printreg %rcx
	andq %rcx, %rcx         # Test val = 0
	jne Loop                # If !0, goto Loop
Done:
	rmmovq %rdx,A(%r14)	# save updated A
	printall
	printmem A(%r14)
    ret
	halt
A:	0xbebafeca
B:	0x1234

