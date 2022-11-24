  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #

.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm

  #
  # Please add your own code below this line
  #

  # timetemplate.asm
  # Written 2015 by F Lundevall
  # Copyright abandonded - this file is in the public domain.

	.data
	.align 2
mytime:	.word 0x5957
timstr:	.ascii "text more text lots of text\0"
ninestr: .ascii "NINE\0"
	.text
	
.global delay
.global time2string
.global hexasc
.global enable_interrupt

main:
	# print timstr
	la	$a0,timstr
	li	$v0,4
	syscall
	nop
	# wait a little
	li	$a0,10000
	jal	delay
	nop
	# call tick
	la	$a0,mytime
	jal	tick
	nop
	# call your function time2string
	la	$a0,timstr
	la	$t0,mytime
	lw	$a1,0($t0)
	jal	time2string
	nop
	# print a newline
	li	$a0,10
	li	$v0,11
	syscall
	nop
	# go back and do it all again
	j	main
	nop
# tick: update time pointed to by $a0
tick:	lw	$t0,0($a0)	# get time
	addiu	$t0,$t0,1	# increase
	andi	$t1,$t0,0xf	# check lowest digit
	sltiu	$t2,$t1,0xa	# if digit < a, okay
	bnez	$t2,tiend
	nop
	addiu	$t0,$t0,0x6	# adjust lowest digit
	andi	$t1,$t0,0xf0	# check next digit
	sltiu	$t2,$t1,0x60	# if digit < 6, okay
	bnez	$t2,tiend
	nop
	addiu	$t0,$t0,0xa0	# adjust digit
	andi	$t1,$t0,0xf00	# check minute digit
	sltiu	$t2,$t1,0xa00	# if digit < a, okay
	bnez	$t2,tiend
	nop
	addiu	$t0,$t0,0x600	# adjust digit
	andi	$t1,$t0,0xf000	# check last digit
	sltiu	$t2,$t1,0x6000	# if digit < 6, okay
	bnez	$t2,tiend
	nop
	addiu	$t0,$t0,0xa000	# adjust last digit
tiend:	sw	$t0,0($a0)	# save updated result
	jr	$ra		# return
	nop

  # you can write your code for subroutine "hexasc" below this line

hexasc:

	add $t0, $a0, $zero
	li $t1, 0xF
	and $t0, $t1, $t0 # zero bits until 4 least significant
	
	li $t2, 9
	
	ble $t0, $t2, digit
	nop
	addi $t0, $t0, 7
	digit:
	addi $t0, $t0, 48
	
	add $v0, $t0, $zero
	
	jr $ra
	nop
 	
time2string:
	# Parameters (two): Register $a0 contains the address of an area in memory, suitably large for the
	# output from time2string. The 16 least significant bits of register $a1 contains time-info, organized
	# as four NBCD-coded digits of 4 bits each. All other bits in register $a1 can have any value and
	# must be ignored. Example: register $a0 can contain the address 0x100100017, and register $a1 can
	# contain the value 0x00001653.
	# Return value: None.
	# Required action: The following sequence of six characters must be written to the area in memory
	# pointed to by register $a0.
	
	# Save registers

	PUSH $ra
	PUSH $s1
	PUSH $s2
	
	# ignore all but 16 least significant bits
	add $t0, $a1, $zero
	andi $s2, $t0, 0xFFFF
	
	# $a0 (memory adress) is stored in $s1, so that $a0 is free
	add $s1, $a0, $zero
	
	srl $a0, $s2, 12
	jal hexasc
	nop
	sb $v0, 0($s1)
	
	srl $a0, $s2, 8
	jal hexasc
	nop
	sb $v0, 1($s1)
	
	li $v0, 0x3A
	sb $v0, 2($s1)
	
	srl $a0, $s2, 4
	jal hexasc
	nop
	sb $v0, 3($s1)
	
	srl $a0, $s2, 0
	addi $t7, $zero, 9
	
	add $t6, $a0, 0
	li $t5, 0xF
	and $t6, $t5, $t6 
		beq $t6, $t7, nine
	nop

	jal hexasc
	nop
	sb $v0, 4($s1)
	sb $zero, 5($s1) # null character
	j notnine
	nop
	
	nine:

	addi $t7, $zero, 78
	sb $t7, 4($s1)
	addi $t7, $zero, 73
	sb $t7, 5($s1)
	addi $t7, $zero, 78
	sb $t7, 6($s1)
	addi $t7, $zero, 69
	sb $t7, 7($s1)
	
	sb $zero, 8($s1) # null character

	notnine:
	
	POP $s2
	POP $s1
	POP $ra
	jr $ra
	nop
	
	  
delay:
	# $a0 is the time in ms
	
	li $t1, 7000
	
	delayloop:
		ble $a0, $zero, delaybreak
		nop
		addi $a0, $a0, -1
		add $t0, $zero, $zero
		delayinner:
			addi $t0, $t0, 1
			bne $t0, $t1, delayinner
			nop
		
		j delayloop
		nop
	delaybreak:
	jr $ra
	nop


enable_interrupt:
	ei
	jr $ra