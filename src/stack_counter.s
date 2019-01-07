main:
mov r2,#0
mov r1,#8 @ recursion amount
bl count
b stop
stop:
  mov r8,#0
  str r8,[r4,#28]
  b stop
count:
  push {lr}
  cmp r2,r1
  bge cntend
  cmp r2,#32
  bge cntend
  bl light
  add r2,r2,#1
  bl count
cntend:
  pop {pc}
light:
  push {lr}
  mov r3,r2
  mov r6,#0
  mov r8,#0
  ldr r4,=0x20200000
0th:
  tst r3,#1
  beq 1st
  mov r5,#1
  lsl r5,#9
  add r6,r6,r5
  mov r7,#1
  lsl r7,#23 @ gpio 23 - 0th bit
  add r8,r8,r7
1st:
  lsr r3,#1
  tst r3,#1
  beq 2nd
  mov r5,#1
  lsl r5,#12
  add r6,r6,r5
  mov r7,#1
  lsl r7,#24 @ gpio 24 - 1st bit
  add r8,r8,r7
2nd:
  lsr r3,#1
  tst r3,#1
  beq 3rd
  mov r5,#1
  lsl r5,#15
  add r6,r6,r5
  mov r7,#1
  lsl r7,#25 @ gpio 25 - 2nd bit
  add r8,r8,r7
3rd:
  lsr r3,#1
  tst r3,#1
  beq 4th
  mov r5,#1
  lsl r5,#21
  add r6,r6,r5
  mov r7,#1
  lsl r7,#27 @ gpio 27 - 3rd bit
  add r8,r8,r7
4th:
  lsr r3,#1
  tst r3,#1
  beq add
  mov r5,#1
  lsl r5,#6
  add r6,r6,r5
  mov r7,#1
  lsl r7,#22 @ gpio 22 - 4th bit
  add r8,r8,r7
add:
  str r6,[r4,#8]
lightup:
  str r8,[r4,#40]
  mov r9,#0x4F0000
wait1:
  sub r9,r9,#1
  cmp r9,#0
  bne wait1
  str r8,[r4,#28]
  mov r9,#0x1F0000
wait2:
  sub r9,r9,#1
  cmp r9,#0
  bne wait2
lightend:
  pop {lr}
  bx lr
