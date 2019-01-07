b main
foreColour:
.int 65535
graphicsAddress:
.int 0
.align 12
FrameBufferInfo:
.int 1024	@ #0 Width
.int 768	@ #4 Height
.int 1024	@ #8 vWidth
.int 768	@ #12 vHeight
.int 0		@ #16 GPU - Pitch
.int 24		@ #20 Bit Dpeth
.int 0		@ #24 X
.int 0		@ #28 Y
.int 0		@ #32 GPU - Pointer
.int 0		@ #36 GPU - Size
.align 2
main:

ldr sp,=0xA3BC

mov r0,#1024
mov r1,#768
mov r2,#16
bl InitialiseFrameBuffer

teq r0,#0
bne noError$

error$:
b error$
noError$:

mov r4,r0
fbInfoAddr .req r4

bl SetGraphicsAddress

length .req r7
direction .req r8 @ 0 = N, 1 = E, 2 = S, 3 = W
colour .req r10
x .req r9
y .req r11
mov length,#760
mov direction,#1
mov x,#100
mov y,#10

mov r10,#0
mov r2,#0
mov r3,#0

mov colour,#0
bl SetForeColour
bl render$

b stop

render$:
sub colour,#1
bl SetForeColour

push {r4,r5,r6,r7,r8,r9,r10,r11,lr}
cmp length,#0
beq finish

mov r0,x
mov r1,y

cmp direction,#0
beq move_north

cmp direction,#1
beq move_east

cmp direction,#2
beq move_south

cmp direction,#3
beq move_west

move_north:
sub r3,y,length
mov r2,x
b continue
move_south:
add r3,y,length
mov r2,x
b continue
move_east:
add r2,x,length
mov r3,y
b continue
move_west:
sub r2,x,length
mov r3,y
b continue

continue:

bl DrawLine

mov x,r2
mov y,r3
sub length,length,#10
bl Rotate
@ Delay
ldr r12,=0x3F0000
wait:
sub r12,#1
cmp r12,#0
bne wait
bl render$

finish:
pop {r4,r5,r6,r7,r8,r9,r10,r11,pc}

stop:
b stop

@
@	drawing.s
@

SetForeColour:
cmp r0,#0x10000
movhi pc,lr
moveq pc,lr

ldr r1,=foreColour
str r0,[r1]
mov pc,lr

SetGraphicsAddress:
ldr r1,=graphicsAddress
str r0,[r1]
mov pc,lr

DrawPixel:
ldr r2,=graphicsAddress
px .req r0
py .req r1
addr .req r2
ldr addr,[addr]

height .req r3
ldr height,[addr,#4]
sub height,#1
cmp py,height
movhi pc,lr
.unreq height

width .req r3
ldr width,[addr,#0]
sub width,#1
cmp px,width
movhi pc,lr

ldr addr,[addr,#32]
add width,#1
mla px,py,width,px
.unreq width
.unreq py
add addr, px,lsl #1
.unreq px

fore .req r3
ldr fore,=foreColour
ldr fore,[fore]

str fore,[addr]
.unreq fore
.unreq addr
mov pc,lr

DrawLine:
push {r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
x0 .req r9
x1 .req r10
y0 .req r11
y1 .req r12

mov x0,r0
mov x1,r2
mov y0,r1
mov y1,r3

dx .req r4
dyn .req r5
sx .req r6
sy .req r7
err .req r8

cmp x0,x1
subgt dx,x0,x1
movgt sx,#-1
suble dx,x1,x0
movle sx,#1

cmp y0,y1
subgt dyn,y1,y0
movgt sy,#-1
suble dyn,y0,y1
movle sy,#1

add err,dx,dyn
add x1,sx
add y1,sy

pixelLoop$:
teq x0,x1
teqne y0,y1
popeq {r4,r5,r6,r7,r8,r9,r10,r11,r12,pc}

mov r0,x0
mov r1,y0
push {r0-r3}
bl DrawPixel
pop {r0-r3}

lsl err #1
cmp dyn, err
addle err,dyn
addle x0,sx

lsl err #1
cmp dx, err
addge err,dx
addge y0,sy

b pixelLoop$

.unreq x0
.unreq x1
.unreq y0
.unreq y1
.unreq dx
.unreq dyn
.unreq sx
.unreq sy
.unreq err


@
@
@	Rotate.s
@
@

Rotate:
push {r8, lr}
direction .req r8

cmp direction,#0
beq set_east
cmp direction,#1
beq set_south
cmp direction,#2
beq set_west
cmp direction,#3
beq set_north

set_north:
mov direction,#0
b return
set_east:
mov direction,#1
b return
set_south:
mov direction,#2
b return
set_west:
mov direction,#3
b return

@ Default case
b set_east

return:
push {r8, lr}
.unreq direction
mov pc,lr

@
@
@	frameBuffer.s
@
@

InitialiseFrameBuffer:
cmp r0,#4096
width .req r0
height .req r1
bitDepth .req r2
cmpls height,#4096
cmpls bitDepth,#32
result .req r0
movhi result,#0
movhi pc,lr

push {r4,lr}
fbInfoAddr .req r4
ldr fbInfoAddr,=FrameBufferInfo
str width,[r4,#0]
str height,[r4,#4]
str width,[r4,#8]
str height,[r4,#12]
str bitDepth,[r4,#20]
.unreq width
.unreq height
.unreq bitDepth

mov r0,fbInfoAddr
push {r3}
ldr r3,=0x4
add r0,r3
pop {r3}
mov r1,#1
bl MailboxWrite

mov r0,#1
bl MailboxRead

teq result,#0
movne result,#0
popne {r4,pc}

mov result,fbInfoAddr
pop {r4,pc}
.unreq result
.unreq fbInfoAddr


@
@
@	mailbox.s
@
@

GetMailboxBase:
ldr r0,=0x2000B880
mov pc,lr

MailboxRead:
and r3,r0,#0xf
mov r2,lr
bl GetMailboxBase
mov lr,r2

rightmail$:
wait1$:
ldr r2,[r0,#24]
push {r3}
ldr r3,=0x4
tst r2,r3
pop {r3}
bne wait1$

ldr r1,[r0,#0]
push {r4}
ldr r4,=0xf
and r2,r1,r4
pop {r4}
teq r2,r3
bne rightmail$
push {r3}
ldr r3,=0xfffffff
and r0,r1,r3
pop {r3}
mov pc,lr

MailboxWrite:
and r2,r1,#0xf
push {r3}
ldr r3,=0xfffffff
and r1,r0,r3
pop {r3}
orr r1,r2
mov r2,lr
bl GetMailboxBase
mov lr,r2

wait2$:
ldr r2,[r0,#24]
tst r2,#0x8
bne wait2$

str r1,[r0,#32]
mov pc,lr
