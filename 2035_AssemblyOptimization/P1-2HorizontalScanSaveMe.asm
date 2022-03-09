#     T o p  o f  P i l e
#
#
# This routine finds the color of the part on top of a pile.
#
# Name: Conner Awald
# Date: October 2, 2019

.data
Pile:  .alloc	1024
Primes: .word 0,0 
.text

TopOfPile:	addi	$1, $0, Pile		# point to array base
			swi	545			# generate pile
			addi $1, $0, 0 #Start the offset at the second line since the first will always be empty
NextLine:   addi $1, $1, 63
BackToLoop: addi $1, $1, 1 #Increment for loop
MainLoop:	lb $2, Pile($1) #Get the current color
			beq $2, $0, BackToLoop #Repeat loop if current color is black 
FoundColor: addi $5, $1, 1
			lb $5, Pile($5)
			beq $5, $0, BackToLoop
            addi $5, $1, 64 #Reference one below
			lb $5, Pile($5) #See what color is below
			beq $5, $2, BackToLoop #If below is the same, we have a vertical line and can move on
Horizontal: addi $5, $1, -64
			lb $5, Pile($5)
			beq $5, $0, HorizontalCont#If color above is 0 then we can keep going
			beq $5, $2, HorizontalCont#If color above is the same color, we can keep going
			addi $5, $1, 64
			lb $5, Pile($5)
			bne $5, $0, Failed #If the color above is non-zero after having a color above, we have a fail
HorizontalCont:	addi $1, $1, 1 #Check next one
			lb $5, Pile($1)
			beq $5, $2, Horizontal #This is if the color to the right is the same as the line color
			beq $5, $0, NextLine #This is if the color to the right is 0, we have found the end of the line
			addi $1, $1, 1
			lb $5, Pile($1)
			bne $5, $2, NextLine
Failed:		lb $3, Primes($5)
			bne $3, $0, HorizontalCont
			sb $5, Primes($5)
			lb $3, Primes($0)
			addi $3, $3, 1
			slti $5, $3, 6
			sb $3, Primes($0) 
			bne $5, $0, HorizontalCont			
CheckStart:	addi $1, $0, 0
CheckLoop:  addi $1, $1, 1
			lb $2, Primes($1)
			bne $2, $0, CheckLoop
        	addi    $2, $1, 0               # TEMP: guess the first color
			swi	546			# submit answer and check
			jr	$31			# return to caller
