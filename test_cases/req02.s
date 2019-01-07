.int 31
.half 15
a .req r2
b .req r1
mov a,#5
wait:
sub a,a,#1
mov b,#10
wait1:
sub b,b,#1
cmp b,a
bne wait1
cmp a,#0
bne wait
.unreq a
.unreq b
