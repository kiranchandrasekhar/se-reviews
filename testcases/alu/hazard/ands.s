	.arch armv8-a
	.text
	.align	2
start:

    // Put values into the registers
    movz x0, #79
    movz x1, #139

    ands x2, x0, x1

    // Print x2, should be 11
    eor 	x5, x5, x5
	mvn 	x5, x5
	stur	x2, [x5]

	ret
	.size	start, .-start
	.ident	"GCC: (Ubuntu/Linaro 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
