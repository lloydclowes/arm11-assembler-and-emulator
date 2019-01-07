
/******************************************************************************
*
*	main.s
*
******************************************************************************/

/*
* .globl is a directive to our assembler, that tells it to export this symbol
* to the elf file. Convention dictates that the symbol _start is used for the
* entry point, so this all has the net effect of setting the entry point here.
* Ultimately, this is useless as the elf itself is not used in the final
* result, and so the entry point really doesn't matter, but it aids clarity,
* allows simulators to run the elf, and also stops us getting a linker warning
* about having no entry point.
*/
.section .init
.globl _start
_start:

/*
* Branch to the actual main code.
*/
b main

/*
* This command tells the assembler to put this code with the rest.
*/
.section .text

/*
* main is what we shall call our main operating system method. It never
* returns, and takes no parameters.
* C++ Signature: void main(void)
*/
main:

/*
* Set the stack point to 0x8000.
*/
	mov sp,#0x8000

/*
* Setup the screen.
*/
	mov r0,#1024
	mov r1,#768
	mov r2,#16
	bl InitialiseFrameBuffer

/*
* Check for a failed frame buffer.
*/
	teq r0,#0
	bne noError$

	mov r0,#16
	mov r1,#1
	bl SetGpioFunction

	mov r0,#16
	mov r1,#0
	bl SetGpio

	error$:
		b error$

	noError$:

	fbInfoAddr .req r4
	mov fbInfoAddr,r0

/* NEW
* Let our drawing method know where we are drawing to.
*/
	bl SetGraphicsAddress

	length .req r7
	direction .req r8 /* 0 = N, 1 = E, 2 = S, 3 = W */
	colour .req r10
	x .req r9
	y .req r11
	mov length,#100
  mov direction,#2
	mov x,#512
  mov y,#700

	mov r10,#0
  mov r2,#0
  mov r3,#0

  mov colour,#0
  bl SetForeColour
  bl tree$

  b stop

tree$:
  push {r4,r5,r6,r7,r8,r9,r10,r11,lr}
	sub length,#10
  cmp length,#5
  ble finish

  /* Initialize coordinates */
	mov r0,x
	mov r1,y
  /* remember the coordinate */
  mov r9,x
  mov r11,y

  /* Set x2 and y2 using length and direction */

  cmp direction,#0
  beq move_east

	cmp direction,#1
	beq move_northeast

	cmp direction,#2
	beq move_north

	cmp direction,#3
	beq move_northwest

	cmp direction,#4
  beq move_west

	cmp direction,#5
	beq move_southwest

  cmp direction,#6
  beq move_south

	cmp direction,#7
	beq move_southeast

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
  move_northeast:
    sub r3,y,length
    add r2,x,length
    b continue
  move_northwest:
    sub r3,y,length
    sub r2,x,length
    b continue
	move_southwest:
		add r3,y,length
		sub r2,x,length
		b continue
	move_southeast:
		add r3,y,length
		add r2,x,length
		b continue

  continue:
  bl DrawLine
	mov x,r2
	mov y,r3
  bl RotateRight
	@ Delay
  mov r12,#0x10000
  wait1:
    sub r12,#1
    cmp r12,#0
    bne wait1
  bl tree$
  bl RotateLeft
	bl RotateLeft
	@ Delay
  mov r12,#0x10000
  wait2:
    sub r12,#1
    cmp r12,#0
    bne wait2
  bl tree$

  mov x,r9
  mov y,r11

  finish:
  pop {r4,r5,r6,r7,r8,r9,r10,r11,pc}

stop:
  b stop

/******************************************************************************
*
*	drawing.s
*
******************************************************************************/

/* NEW
* The foreColour is the colour which all our methods will draw shapes in.
* C++ Signature: short foreColour;
*/
.section .data
.align 1
foreColour:
	.hword 0xFFFF

/* NEW
* graphicsAddress stores the address of the frame buffer info structure.
* C++ Signature: FrameBuferDescription* graphicsAddress;
*/
.align 2
graphicsAddress:
	.int 0

/* NEW
* SetForeColour changes the current drawing colour to the 16 bit colour in r0.
* C++ Signature: void SetForeColour(u16 colour);
*/
.section .text
.globl SetForeColour
SetForeColour:
	cmp r0,#0x10000
	movhi pc,lr
	moveq pc,lr

	ldr r1,=foreColour
	strh r0,[r1]
	mov pc,lr

/* NEW
* SetGraphicsAddress changes the current frame buffer information to
* graphicsAddress;
* C++ Signature: void SetGraphicsAddress(FrameBuferDescription* value);
*/
.globl SetGraphicsAddress
SetGraphicsAddress:
	ldr r1,=graphicsAddress
	str r0,[r1]
	mov pc,lr

/* NEW
* DrawPixel draws a single pixel to the screen at the point in (r0,r1).
* C++ Signature: void DrawPixel(u32x2 point);
*/
.globl DrawPixel
DrawPixel:
	px .req r0
	py .req r1

	addr .req r2
	ldr addr,=graphicsAddress
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
	ldrh fore,[fore]

	strh fore,[addr]
	.unreq fore
	.unreq addr
	mov pc,lr

/* NEW
* DrawLine draws a line between two points given in (r0,r1) and (r2,r3).
* Uses Bresenham's Line Algortihm
* C++ Signature: void DrawLine(u32x2 p1, u32x2 p2);
*/
.globl DrawLine
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
	dyn .req r5 /* Note that we only ever use -deltay, so I store its negative for speed. (hence dyn) */
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

		cmp dyn, err,lsl #1
		addle err,dyn
		addle x0,sx

		cmp dx, err,lsl #1
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

/******************************************************************************
*
*	Rotate.s
*
******************************************************************************/
.globl RotateRight
RotateRight:
	add direction,direction,#1
	cmp direction,#8
	subeq direction,direction,#8
  mov pc,lr

.globl RotateLeft
RotateLeft:
	sub direction,direction,#1
	cmp direction,#-1
	addeq direction,direction,#8
  mov pc,lr

/******************************************************************************
*
*	gpio.s
*
******************************************************************************/

/* NEW
* According to the EABI, all method calls should use r0-r3 for passing
* parameters, should preserve registers r4-r8,r10-r11,sp between calls, and
* should return values in r0 (and r1 if needed).
* It does also stipulate many things about how methods should use the registers
* and stack during calls, but we're using hand coded assembly. All we need to
* do is obey the start and end conditions, and if all our methods do this, they
* would all work from C.
*/

/* NEW
* GetGpioAddress returns the base address of the GPIO region as a physical address
* in register r0.
* C++ Signature: void* GetGpioAddress()
*/
.globl GetGpioAddress
GetGpioAddress:
	gpioAddr .req r0
	ldr gpioAddr,=0x20200000
	mov pc,lr
	.unreq gpioAddr

/* NEW
* SetGpioFunction sets the function of the GPIO register addressed by r0 to the
* low  3 bits of r1.
* C++ Signature: void SetGpioFunction(u32 gpioRegister, u32 function)
*/
.globl SetGpioFunction
SetGpioFunction:
    pinNum .req r0
    pinFunc .req r1
	cmp pinNum,#53
	cmpls pinFunc,#7
	movhi pc,lr

	push {lr}
	mov r2,pinNum
	.unreq pinNum
	pinNum .req r2
	bl GetGpioAddress
	gpioAddr .req r0

	functionLoop$:
		cmp pinNum,#9
		subhi pinNum,#10
		addhi gpioAddr,#4
		bhi functionLoop$

	add pinNum, pinNum,lsl #1
	lsl pinFunc,pinNum


	mask .req r3
	mov mask,#7					/* r3 = 111 in binary */
	lsl mask,pinNum				/* r3 = 11100..00 where the 111 is in the same position as the function in r1 */
	.unreq pinNum

	mvn mask,mask				/* r3 = 11..1100011..11 where the 000 is in the same poisiont as the function in r1 */
	oldFunc .req r2
	ldr oldFunc,[gpioAddr]		/* r2 = existing code */
	and oldFunc,mask			/* r2 = existing code with bits for this pin all 0 */
	.unreq mask

	orr pinFunc,oldFunc			/* r1 = existing code with correct bits set */
	.unreq oldFunc

	str pinFunc,[gpioAddr]
	.unreq pinFunc
	.unreq gpioAddr
	pop {pc}

/* NEW
* SetGpio sets the GPIO pin addressed by register r0 high if r1 != 0 and low
* otherwise.
* C++ Signature: void SetGpio(u32 gpioRegister, u32 value)
*/
.globl SetGpio
SetGpio:
    pinNum .req r0
    pinVal .req r1

	cmp pinNum,#53
	movhi pc,lr
	push {lr}
	mov r2,pinNum
    .unreq pinNum
    pinNum .req r2
	bl GetGpioAddress
    gpioAddr .req r0

	pinBank .req r3
	lsr pinBank,pinNum,#5
	lsl pinBank,#2
	add gpioAddr,pinBank
	.unreq pinBank

	and pinNum,#31
	setBit .req r3
	mov setBit,#1
	lsl setBit,pinNum
	.unreq pinNum

	teq pinVal,#0
	.unreq pinVal
	streq setBit,[gpioAddr,#40]
	strne setBit,[gpioAddr,#28]
	.unreq setBit
	.unreq gpioAddr
	pop {pc}


/******************************************************************************
*
*	frameBuffer.s
*
******************************************************************************/

/*
* When communicating with the graphics card about frame buffers, a message
* consists of a pointer to the structure below. The comments explain what each
* member of the structure is.
* The .align 12 is necessary to ensure correct communication with the GPU,
* which expects page alignment.
* C++ Signature:
* struct FrameBuferDescription {
*  u32 width; u32 height; u32 vWidth; u32 vHeight; u32 pitch; u32 bitDepth;
*  u32 x; u32 y; void* pointer; u32 size;
* };
* FrameBuferDescription FrameBufferInfo =
*		{ 1024, 768, 1024, 768, 0, 24, 0, 0, 0, 0 };
*/
.section .data
.align 12
.globl FrameBufferInfo
FrameBufferInfo:
.int 1024	/* #0 Width */
.int 768	/* #4 Height */
.int 1024	/* #8 vWidth */
.int 768	/* #12 vHeight */
.int 0		/* #16 GPU - Pitch */
.int 24		/* #20 Bit Dpeth */
.int 0		/* #24 X */
.int 0		/* #28 Y */
.int 0		/* #32 GPU - Pointer */
.int 0		/* #36 GPU - Size */

/*
* InitialiseFrameBuffer creates a frame buffer of width and height specified in
* r0 and r1, and bit depth specified in r2, and returns a FrameBuferDescription
* which contains information about the frame buffer returned. This procedure
* blocks until a frame buffer can be created, and so is inapropriate on real
* time systems. While blocking, this procedure causes the OK LED to flash.
* If the frame buffer cannot be created, this procedure treturns 0.
* C++ Signature: FrameBuferDescription* InitialiseFrameBuffer(u32 width,
*		u32 height, u32 bitDepth)
*/
.section .text
.globl InitialiseFrameBuffer
InitialiseFrameBuffer:
width .req r0
height .req r1
bitDepth .req r2
cmp width,#4096
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
add r0,#0x40000000
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


/******************************************************************************
*
*	mailbox.s
*
******************************************************************************/

/* NEW
* GetMailboxBase returns the base address of the mailbox region as a physical
* address in register r0.
* C++ Signature: void* GetMailboxBase()
*/
.globl GetMailboxBase
GetMailboxBase:
	ldr r0,=0x2000B880
	mov pc,lr

/* NEW
* MailboxRead returns the current value in the mailbox addressed to a channel
* given in the low 4 bits of r0, as the top 28 bits of r0.
* C++ Signature: u32 MailboxRead(u8 channel)
*/
.globl MailboxRead
MailboxRead:
	and r3,r0,#0xf
	mov r2,lr
	bl GetMailboxBase
	mov lr,r2

	rightmail$:
		wait1$:
			ldr r2,[r0,#24]
			tst r2,#0x40000000
			bne wait1$

		ldr r1,[r0,#0]
		and r2,r1,#0xf
		teq r2,r3
		bne rightmail$

	and r0,r1,#0xfffffff0
	mov pc,lr

/* NEW
* MailboxWrite writes the value given in the top 28 bits of r0 to the channel
* given in the low 4 bits of r1.
* C++ Signature: void MailboxWrite(u32 value, u8 channel)
*/
.globl MailboxWrite
MailboxWrite:
	and r2,r1,#0xf
	and r1,r0,#0xfffffff0
	orr r1,r2
	mov r2,lr
	bl GetMailboxBase
	mov lr,r2

	wait2$:
		ldr r2,[r0,#24]
		tst r2,#0x80000000
		bne wait2$

	str r1,[r0,#32]
	mov pc,lr

/******************************************************************************
*
	* systemTimer.s
*
******************************************************************************/

/*
* The system timer runs at 1MHz, and just counts always. Thus we can deduce
* timings by measuring the difference between two readings.
*/

/*
* GetSystemTimerBase returns the base address of the System Timer region as a
* physical address in register r0.
* C++ Signature: void* GetSystemTimerBase()
*/
.globl GetSystemTimerBase
GetSystemTimerBase:
	ldr r0,=0x20003000
	mov pc,lr

/*
* GetTimeStamp gets the current timestamp of the system timer, and returns it
* in registers r0 and r1, with r1 being the most significant 32 bits.
* C++ Signature: u64 GetTimeStamp()
*/
.globl GetTimeStamp
GetTimeStamp:
	push {lr}
	bl GetSystemTimerBase
	ldrd r0,r1,[r0,#4]
	pop {pc}

/*
* Wait waits at least a specified number of microseconds before returning.
* The duration to wait is given in r0.
* C++ Signature: void Wait(u32 delayInMicroSeconds)
*/
.globl Wait
Wait:
	delay .req r2
	mov delay,r0
	push {lr}
	bl GetTimeStamp
	start .req r3
	mov start,r0

	loop$:
		bl GetTimeStamp
		elapsed .req r1
		sub elapsed,r0,start
		cmp elapsed,delay
		.unreq elapsed
		bls loop$

	.unreq delay
	.unreq start
	pop {pc}
