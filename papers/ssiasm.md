# soundtracker instrument assembly

this is an advanced mode in where a custom program runs to handle the instrument.

it is based on a stack machine.

## details

- fully stack-oriented instruction set
- stack size is 8 items
- 16-item scratchpad
- works on 32-bit integer and 16-bit fixed-point
- instructions are 64-bit but packed for storage

## instruction set

opcode | sym | desc
-------|-----|----------------------
add    |  +  | add two items
sub    |  -  | subtract two items
mul    |  *  | multiply two items
div    |  /  | divide two items, divisor first
mod    |  %  | remainder of division, divisor first
mulfi  | *fi | multiply int with fixed
mulff  | *ff | multiply fixed with fixed
divfi  | /fi | divide int with fixed
divff  | /ff | divide fixed with fixed
and    |  &  | bitwise and
or     |  |  | bitwise or
xor    |  ^  | bitwise xor
not    |  ~  | bitwise not
neg    |  -  | negative
sal    | <<  | shift left
sar    | >>  | shift right
rol    | <<. | rotate left
ror    | >>. | rotate right
push   |  <  | push value from scratchpad
pushc  |  <  | push value from soundchip
pushi  |  <  | push value immediate
pop    |  >  | pop value to scratchpad
popc   |  >  | pop value to soundchip
drop   |  >  | pop value to nowhere
dup    |  #  | duplicate value
clr    |  .  | empty stack
bra    | ^^^ | branch always
beq    | ^== | branch if zero
bne    | ^!= | branch if not zero
bpl    | ^>  | branch if positive
bmi    | ^<  | branch if negative
off    |  $  | select channel by offset
chan   |  $  | select channel by index
wait   | --- | wait ticks
ret    | === | stop execution
brk    | !!! | break here
nop    | ... | no operation

## soundchip variables

addr | value
-----|-----------
$00  | channel variables (32)
$20  | note frequency
$21  | note in fixed
$22  | unprocessed note
$23  | unprocessed note in fixed
$24  | volume
$25  | unprocessed volume
$26  | panning
$27  | unprocessed panning
$28  | note on
$29  | current tick in pattern
$2a  | current tick in song

# screw this document

yep, this is a lot of nonsense I wrote the other day.