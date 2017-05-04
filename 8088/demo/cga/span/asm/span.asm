cpu 8086

  mov ax,4
  int 0x10
  mov di,spanBuffer0
  call clearSpanBuffer
  mov di,spanBuffer1
  call clearSpanBuffer
frameLoop:
  ; Update rotation angles

  mov bx,[theta]
  add bx,[dTheta]
  mov [theta],bx
  add bx,bx
  mov si,[phi]
  add si,[dPhi]
  mov [phi],si
  add si,si


  ; Compute scaled rotation matrix elements

  ; TODO: We could replace this with prescaled sin, cos, half-sin and half-cos tables at cost of additional 20kB of tables
  ;   sin*s.x
  ;   sin*s.y/2
  ;   sin/2
  ;   sin*s.y
  ;   sin

  mov ax,[sine + bx]  ; sin(theta)
  mov di,ax
  mov cx,0x7f5c  ; s.x = 99.5 * 6/5 * 16/15
  imul cx
  mov [xx],ah
  mov [xx+1],dl   ; xx = s.x*sin(theta)

  mov ax,di
  mov bp,[cosine + si] ; cos(phi)
  imul bp
  mov al,ah
  mov ah,dl
  mov cx,0x6a22  ; s.y = 99.5 * 16/15
  imul cx
  mov [yy],ah
  mov [yy+1],dl   ; yy = s.y*sin(theta)*cos(phi)

  xchg ax,di
  mov cx,[sine + si]   ; sin(phi)
  imul cx
  mov [yz],ah
  mov [yz+1],dl   ; yz = s.z*sin(theta)*sin(phi)

  mov ax,[cosine + bx]  ; cos(theta)
  mov di,ax
  neg ax
  imul cx
  mov [xz],ah
  mov [xz+1],dl   ; xz = -cos(theta)*sin(phi)

  mov ax,di
  neg bp
  mov [zz],bp     ; zz = -cos(phi)
  imul bp
  mov al,ah
  mov ah,dl
  mov bp,0x6a22
  imul bp
  mov [xy],ah
  mov [xy+1],dl   ; xy = -s.y*cos(theta)*cos(phi)

  xchg ax,di
  mov bp,0x7f5c
  imul bp
  mov [yx],ah
  mov [yx+1],dl   ; yx = s.x*cos(theta)

  mov ax,0x6a22
  imul cx
  mov [zy],ah
  mov [zy+1],dl   ; zy = s.y*sin(phi);


  ; Transform vertices to screen coordinates
  mov si,[currentShape]
  lodsb
  mov cl,al
  mov ch,0
  lodsw
  mov si,ax
  mov di,vertexBuffer
transformLoop:
  lodsw
  push si
  mov si,ax

  lodsw
  imul word[xx]
  mov bp,ax
  mov bx,dx
  lodsw
  imul word[yx]
;  add bp,ax
;  adc bx,dx
;  lodsw
;  imul word[zx]
  add ax,bp
  adc dx,bx
  stosw
  mov ax,dx
  stosw
  sub si,4

  lodsw
  imul word[xy]
  mov bp,ax
  mov bx,dx
  lodsw
  imul word[yy]
  add bp,ax
  adc bx,dx
  lodsw
  imul word[zy]
  add ax,bp
  adc dx,bx
  stosw
  mov ax,dx
  stosw
  sub si,6

  lodsw
  imul word[xz]
  mov bp,ax
  mov bx,dx
  lodsw
  imul word[yz]
  add bp,ax
  adc bx,dx
  lodsw
  imul word[zz]
  add ax,bp
  adc dx,bx
  stosw
  mov ax,dx
  stosw

  ; xx xx xx xx yy yy yy yy zz zz zz zz
  ; -c    -a    -8    -6       -3

  mov bx,[di-3]
  mov ax,[di-0x0c]
  mov dx,[di-0x0a]
  idiv bx
  mov [di-0x0c],ax
  mov ax,[di-8]
  mov dx,[di-6]
  idiv bx
  mov [di-0x08],ax

  loop transformLoop


  ; Draw faces into span buffers
  mov si,[currentShape]
  add si,3
  lodsb
  mov cl,al
  mov ch,0
  lodsw
  mov si,ax
faceLoop:
  lodsw
  mov [colour],ax
  lodsw
  push cx
  xchg ax,cx
  push cx
  mov bx,[si]
  mov di,[bx]      ; p0.x
  mov dx,[bx+4]    ; p0.y
  mov bx,[si+2]
  mov cx,[bx]      ; p1.x
  mov ax,[bx+4]    ; p1.y
  mov bx,[si+4]
  mov bp,[bx]      ; p2.x
  mov bx,[bx+4]    ; p2.y
  shr cx,1
  shr bp,1
  shr di,1
  shr dx,1
  shr ax,1
  shr bx,1

  sub cx,di        ; p1.x - p0.x
  sub bp,di        ; p2.x - p0.x
  sub ax,dx        ; p1.y - p0.y
  sub bx,dx        ; p2.y - p0.y
  imul bp          ; (p2.x - p0.x)*(p1.y - p0.y)
  mov bp,ax        ; loword((p2.x - p0.x)*(p1.y - p0.y))
  mov di,dx        ; hiword((p2.x - p0.x)*(p1.y - p0.y))
  mov ax,bx        ; p2.y - p0.y
  imul cx          ; (p2.y - p0.y)*(p1.x - p0.x)
  cmp dx,di        ; hiword((p2.y - p0.y)*(p1.x - p0.x)) <=> hiword((p2.x - p0.x)*(p1.y - p0.y))
  jg skipFace
  jl drawFace
  cmp ax,bp
  jg skipFace
drawFace:
  mov bp,2
  pop cx

drawFacePart:
  push cx
  push bp
  push si
  mov bx,[si]
  mov cx,[bx]       ; cx = a.x
  mov di,[bx+4]     ; di = a.y
  mov bx,[si+bp]
  mov dx,[bx]       ; dx = b.x
  mov ax,[bx+4]     ; ax = b.y
  mov bx,[si+bp+2]
  mov si,[bx]       ; si = c.x
  mov bx,[bx+4]     ; bx = c.y

; slopeLeft dest {dLpatch, dRpatch}, xL {!ax}, xR, dy {!ax, !dx}, x0 {!ax, !dx} {a.x, b.x}, y0 {!ax, !dx}
; Pushes initial x value onto stack
; Stomps ax, dx
%macro slopeLeft 6
%ifnidni %3,ax
  mov ax, %3
%endif
  sub ax, %2
  cmp %4, 0x100
  jae %%largeY
  mul %6
  div %4
  mov dx,0xffff
  push dx
  jmp %%done
%%largeY:
  xor dx, dx
  div %4
  push ax
  mul %6
%done:
  sub ax, %5
  neg ax
  mov [cs:%1-2], ax
%endmacro

; slopeRight dest {dLpatch, dRpatch}, xL {!ax}, xR, dy {!ax, !dx}, x0 {!dx, !dx} {a.x, b.x}, y0 {!ax, !dx}
; Pushes initial x value onto stack
; Stomps ax, dx
%macro slopeRight 6
%ifnidni %3,ax
  mov ax, %3
%endif
  sub ax, %2
  cmp %4, 0x100
  jae %%largeY
  mul %6
  div %4
  mov dx,0xffff
  push dx
  jmp %%done
%%largeY:
  xor dx, dx
  div %4
  push ax
  mul %6
%done:
  add ax, %5
  mov [cs:%1-2], ax
%endmacro

; slope dest {dLpatch, dRpatch}, ux {!ax}, vx {!ax}, dy {!ax, !dx}, y0 {!ax, !dx}
; Pushes initial x value onto stack
; Stomps ax, dx
%macro slope 5
  mov ax, %3
  sub ax, %2
  jc %%left
  cmp %4, 0x100
  jae %%largeY
  mul %5
  div %4
  mov dx,0xffff
  push dx
  add ax, %3
  mov [cs:%1-2], ax
  jmp %%done
%%largeY:
  xor dx, dx
  div %4
  push ax
  mul %5
  add ax, %3
  mov [cs:%1-2], ax
  jmp %%done
%%left
  cmp %4, 0x100
  jae %%largeY
  mul %5
  div %4
  mov dx,0xffff
  push dx
  sub ax, %3
  neg ax
  mov [cs:%1-2], ax
  jmp %%done
%%largeY:
  xor dx, dx
  div %4
  push ax
  mul %5
  sub ax, %3
  neg ax
  mov [cs:%1-2], ax
%done:
%endmacro


  ; Fill triangle
  ; cx = a.x
  ; di = a.y
  ; dx = b.x
  ; ax = b.y
  ; si = c.x
  ; bx = c.y
  mov bp,colour+2
  cmp di,ax
  jle noSwapAB
  xchg di,ax
  xchg cx,dx
noSwapAB:
  cmp ax,bx
  jle noSwapBC
  xchg ax,bx
  xchg dx,si
noSwapBC:
  cmp di,ax
  jle noSwapAB2
  xchg di,ax
  xchg cx,dx
noSwapAB2:
  cmp di,ax
  jne notHorizontalAB
  cmp ax,bx
  je doneTriangle
  cmp cx,dx
  jbe noSwapABx
  xchg cx,dx
noSwapABx:
  mov [bp-0x11],dx   ; coordBX

  add ax,0xff
  mov [bp-8],ah  ; yab = a.y.intCeiling();
  mov ax,bx
  inc ah
  mov [bp-6],ah  ; yc = (c.y + 1).intFloor();

  xchg cx,bx       ; TODO: try switching definitions of bx and cx
  sub cx,di      ; yac = c.y - a.y

  sub di,[bp-9]  ; yab (Note: low byte is kept as 0)
  neg di         ; yaa = yab - a.y

  slope dLpatch, si, bx, cx, di   ; c.x, a.x, yac, yaa
  mov bx,[bp-0x11]
  slope dRpatch, si, bx, cx, di   ; c.x, b.x, yac, yaa
  pop ax
  pop dx
  mov si,[bp-2] ; colour
  mov bx,[bp-8] ; yab
  mov cx,[bp-6] ; yc
  call fillTrapezoid
  jmp doneTriangle

notHorizontalAB:
  mov [bp-0x15],cx   ; coordAX
  mov [bp-0x13],di   ; coordAY
  mov [bp-0x11],dx   ; coordBX
  mov [bp-0xf],ax    ; coordBY
  mov [bp-0xd],si    ; coordCX
  mov [bp-0xb],bx    ; coordCY



  xchg ax,di
  inc ah
  mov [ya],ah  ; ya = (a.y + 1).intFloor();

  mov ax,di
  sub ax,[coordAY]
  mov [bp-8],ax

  cmp di,bx
  jne notHorizontalBC
  cmp dx,si
  jbe noSwapBCx
  xchg dx,si
noSwapBCx:
  xchg ax,di
  inc ah
  mov [ybc],ah  ; ybc = (b.y + 1).intFloor();

  mov bp,[ya-1]  ; Note: low byte is kept as 0
  sub bp,[coordAY]  ; yaa = ya - a.y

  mov [coordBX],dx

  slope dLpatch, [coordBX], cx, [bp-8], bp, di
  slope dRpatch, si, cx, [bp-8], bp, bx
  mov si,[ya]
  mov cx,[ybc]  ; Note: high byte is kept as 0
  mov si,[colour]
  call fillTrapezoid
  jmp doneTriangle






doneTriangle:
  pop si
  pop bp
  pop cx
  inc bp
  inc bp
  loop drawFacePart


skipFace:
  pop ax
  pop cx
  loop faceLoop


  ; vsync
  mov dx,0x3da
vsync:
  in al,dx
  test al,8
  jz vsync


  ; Render deltas


  ; Switch buffers
  mov ax,[spanBuffer]
  mov bx,spanBuffer1 + spanBuffer0
  sub bx,ax
  xchg ax,bx
  mov [spanBuffer],ax
  mov word[cs:spanBufferPatch+2],ax

  ; Clear next buffer
  mov di,ax
  call clearSpanBuffer

  ; Handle keyboard
  mov ah,1
  int 0x16
  test ax,ax
  jz noKey
  mov ah,0
  int 0x16

  cmp al,27
  jne noExit
  mov ax,3
  int 0x10
  mov ax,0x4c00
  int 0x21

noExit:
  cmp al,'n'
  je change
  cmp al,'N'
  jne noChange
  mov ax,[currentShape]
  add ax,6
  cmp ax,endShapes
  jne notEndShape
  mov ax,shapes
notEndShape:
  mov [currentShape],ax
noKey:
  jmp frameLoop

noChange:
  cmp al,' '
  jne noStartStop
  xor byte[autoRotate],1
  jmp noKey

noStartStop:
  cmp ah,0x4b
  jne noLeft
  test byte[autoRotate],1
  jz noLeftAccel
  dec word[dTheta]
  jmp noKey
noLeftAccel:
  dec word[theta]
  jmp noKey

noLeft:
  mov ah,0x4d
  jne noRight
  test byte[autoRotate],1
  jz noRightAccel
  inc word[dTheta]
  jmp noKey
noRightAccel:
  inc word[theta]
  jmp noKey

noRight:
  cmp ah,0x48
  jne noUp
  test byte[autoRotate],1
  jz noUpAccel
  dec word[dPhi]
  jmp noKey
noUpAccel:
  dec word[phi]
  jmp noKey

noDown:
  cmp ah,0x50
  jne noDown
  test byte[autoRotate],1
  jz noDownAccel
  inc word[dPhi]
  jmp noKey
noDownAccel:
  inc word[phi]
  jmp noKey


; Span buffer format
;   word[bx]   = bx+n*2+1 (n = number of entries (not including final sentinel with x == 255))
;   byte[bx+2] = colour of pixel 0
;   byte[bx+3] = first transition position
;   ...
;   byte[bx+n*2+1] = 255

fillTrapezoid:
  ; inputs:
  ;   si = colour pair pointer
  ;   bx = yStart
  ;   cx = yEnd
  ;   dx = _xL
  ;   ax = _xR
  ; stomps: ax, bx, cx, dx, si, di, bp
  push bp
  mov bp,bx
  and bp,1
  add si,bp
  sub cx,bx
  add bx,bx
spanBufferPatch:
  mov bx,[bx+spanBuffer0]
fillTrapezoidLoop:
  cmp dx,ax
  jge skipAddSpan
  push dx
  push ax
  push si
  lodsb
  push cx

  ; addSpan:
  ; inputs:
  ;   al = c
  ;   dh = xL
  ;   ah = xR
  ;   bx = span buffer line pointer + 1
  ; used:
  ;   di = &_s[i]._x = bx+2*i    di+1 = &_s[i]._c
  ;   si = &_s[j+1]._x = bx+2*j+2

  mov cx,[bx-1]

  mov di,bx
.findI:
  inc di
  inc di
  cmp dh,[di]
  jge .findI
  dec di
  dec di

  mov si,bx
.findJ:
  inc si
  inc si
  cmp dh,[si]
  jge .findJ

  cmp al,[di+1]
  jne .differentColourL
  mov dh,[di]
  jmp .doneCheckL
.differentColourL:
  cmp di,bx
  je .doneCheckL
  cmp dh,[di]
  jne .doneCheckL
  cmp al,[di-1]
  jne .doneCheckL
  dec di
  dec di
  mov dh,[di]
.doneCheckL:

  cmp al,[si-1]
  jne .differentColourR
  mov ah,[si]
  jmp .doneCheckR
.differentColourR:
  cmp si,cx
  jae .doneCheckR
  cmp ah,[si]
  jne .doneCheckR
  cmp al,[si+1]
  jne .doneCheckR
  inc si
  inc si
  mov ah,[si]
.doneCheckR:

  lea bp,[si-2]
  sub bp,di                 ; si-2-di = bx+2*j+2-2-bx-2*i = 2*(j-i)
  cmp dh,[di]
  jne .noLeftCoincide
  cmp ah,[si]
  jne .noRightCoincideL

  ; Both edges coincide with an existing edge
  sub cx,bp
  mov [bx-1],cx
  sub cx,di
  inc di
  stosb
  shr cx,1
  rep movsw
  jmp endAddSpan

.noRightCoincideL:
  ; Left edge coincides with an existing edge but right does not
  dec bp
  dec bp
  sub cx,bp
  mov [bx-1],cx
  cmp bp,0
  jl .oMinusOneL
  je .oZeroL
  sub cx,di       ; count = _n+1-i-2 = _n-i-1 = _n-(i+1)
  inc di
  stosw
  dec si          ; &_s[i+1+o]._c == &_s[i+1+j-i-1]._c = &_s[j]._c = si-1
  movsb
  shr cx,1        ; si = &_s[i+2+o] = &_s[i+2+j-i-1] = &_s[1+j]
  dec cx
  rep movsw
  jmp endAddSpan
.oZeroL:
  inc di
  stosw
  jmp endAddSpan
.oMinusOneL:
  xchg cx,di      ; cx = &_s[i], di = &_s[_n]
  lea si,[di+bp]  ; si = &_s[_n + o]
  neg cx
  add cx,si       ; cx = 2*(_n + o - i)
  shr cx,1
  std
  rep movsw       ; di = &_s[_n - _n - o + i] = &_s[i-o] = &_s[i+1]   (last word written was at [di+2])
  cld
  dec di
  stosw
  inc si
  movsb
  jmp endAddSpan

.noLeftCoincide:
  cmp ah,[di]
  jne .noRightCoincide

  ; Right edge coincides with an existing edge but left does not
  dec bp
  dec bp
  sub cx,bp
  mov [bx-1],cx
  cmp bp,0
  jl .oMinusOneR
  je .oZeroR
  inc di
  inc di
  sub cx,di       ; count = _n+1-i-2 = _n-i-1 = _n-(i+1)
  mov [di],dh
  inc di
  stosb
  shr cx,1        ; si = &_s[i+2+o] = &_s[i+2+j-i-1] = &_s[1+j]
  rep movsw
  jmp endAddSpan
.oZeroR:
  inc di
  inc di
  mov [di],dh
  inc di
  stosb
  jmp endAddSpan
.oMinusOneR:
  xchg cx,di      ; cx = &_s[i], di = &_s[_n]
  lea si,[di+bp]  ; si = &_s[_n + o]
  neg cx
  add cx,si       ; cx = 2*(_n + o - i)
  shr cx,1
  inc cx          ; cx = 1 + _n + o - i
  std
  rep movsw
  cld             ; di = &_s[_n - 1 - _n - o + i] = &_s[i-o-1] = &_s[i]   (last word written was at [di+2])
  inc di
  inc di
  mov [di],dh
  inc di
  stosb
  jmp endAddSpan

.noRightCoincide:
  ; Neither side coincides with an existing edge
  sub bp,4
  sub cx,bp
  mov [bx-1],cx
  cmp bp,0
  jl .oMinus
  je .oZero
  inc di
  inc di
  sub cx,di
  mov [di],dh     ; count = _n+1-i-3 = _n-i-2
  inc di
  stosw
  dec si
  movsb                     ; &_s[i+2+o]._c == &_s[i+2+j-i-2] == &_s[j]
  inc di    ; di = &_s[i+3]
  dec cx
  shr cx,1  ; si = &_s[i+3+o] = &_s[i+3+j-i-2] = &_s[j+1]
  rep movsw
  jmp endAddSpan
.oZero:
  mov [di+2],dh
  mov [di+3],ax
  jmp endAddSpan
.oMinus:
  cmp bp,-2
  jge .oMinusOne
  xchg cx,di      ; cx = &_s[i], di = &_s[_n]
  lea si,[di+bp]  ; si = &_s[_n + o] = &_s[_n - 2]
  neg cx
  add cx,si       ; cx = 2*(_n + o - i) = 2*(_n - 2 - i)
  shr cx,1
  std
  rep movsw       ; di = &_s[_n - _n - o + i] = &_s[i-o] = &_s[i+2]   (last word written was at [di+2] which is &_s[i+3])  si = &_s[i]
  movsw           ; di = &_s[i+1]
  cld
  mov [di],dh
  inc di
  stosw
  jmp endAddSpan
.oMinusOne:
  xchg cx,di      ; cx = &_s[i], di = &_s[_n]
  lea si,[di+bp]  ; si = &_s[_n + o]
  neg cx
  add cx,si       ; cx = 2*(_n + o - i)
  shr cx,1
  std
  rep movsw
  cld             ; di = &_s[_n - _n - o + i] = &_s[i-o] = &_s[i+2]   (last word written was at [di+2] which is &_s[i+3])  si = &_s[i]
  dec di
  mov [di-1],dh
  stosw
endAddSpan:

  pop cx
  pop si
  pop ax
  pop dx
skipAddSpan:
  xor si,1
  add dx,9999
dLpatch:
  add ax,9999
dRpatch:
  add bx,spanBufferEntries*2
  loop fillTrapezoidLoop
  pop bp
  ret



colours:
  db 0x00, 0x00
  db 0x55, 0x55
  db 0xaa, 0xaa
  db 0xff, 0xff
  db 0x66, 0x99
  db 0x77, 0xdd
  db 0xbb, 0xee
  db 0x11, 0x44
  db 0x22, 0x88
  db 0x33, 0xcc

%macro vertex 1
  dw vertexBuffer+%1*4
%endmacro

%macro face 4-*
  dw 2*%1+colours,%0-2
  %rotate 1
  %rep %0-1
  vertex %1
  %endrep
%endmacro

%macro shape 4
  db %1
  dw %2
  db %3
  dw %4
%endmacro

vertexBuffer:
  times 20*3 dd 0

cubeVertices:
  dw -107, -107, -107
  dw -107, -107,  107
  dw -107,  107, -107
  dw -107,  107,  107
  dw  107, -107, -107
  dw  107, -107,  107
  dw  107,  107, -107
  dw  107,  107,  107
cubeFaces:
  face 1,   0,  4,  6,  2
  face 2,   4,  5,  7,  6
  face 1,   5,  1,  3,  7
  face 2,   1,  0,  2,  3
  face 3,   2,  6,  7,  3
  face 3,   0,  1,  5,  4

octahedronVertices:
  dw  186,    0,    0
  dw -186,    0,    0
  dw    0,  186,    0
  dw    0, -186,    0
  dw    0,    0,  186
  dw    0,    0, -186
octahedronFaces:
  face 3,   4,  2,  0
  face 2,   5,  0,  2
  face 2,   4,  0,  3
  face 1,   5,  3,  0
  face 2,   4,  1,  2
  face 1,   5,  2,  1
  face 1,   4,  3,  1
  face 3,   5,  1,  3

tetrahedronVertices:
  dw  107,  107,  107
  dw  107, -107, -107
  dw -107,  107, -107
  dw -107, -107,  107
tetrahedronFaces:
  face 4,   1,  2,  3
  face 1,   0,  3,  2
  face 2,   3,  0,  1
  face 3,   2,  1,  0

icosahedronVertices:
  dw  158,   98,    0
  dw -158,   98,    0
  dw  158,  -98,    0
  dw -158,  -98,    0
  dw   98,    0,  158
  dw   98,    0, -158
  dw  -98,    0,  158
  dw  -98,    0, -158
  dw    0,  158,   98
  dw    0, -158,   98
  dw    0,  158,  -98
  dw    0, -158,  -98
icosahedronFaces:
  face 1,   4,  8,  0
  face 2,  10,  5,  0
  face 2,   9,  4,  2
  face 3,   5, 11,  2
  face 2,   8,  6,  1
  face 6,   7, 10,  1
  face 5,   6,  9,  3
  face 2,  11,  7,  3
  face 3,   8, 10,  0
  face 5,  10,  8,  1
  face 1,  11,  9,  2
  face 6,   9, 11,  3
  face 5,   0,  2,  4
  face 6,   2,  0,  5
  face 1,   3,  1,  6
  face 3,   1,  3,  7
  face 6,   4,  6,  8
  face 3,   6,  4,  9
  face 1,   7,  5, 10
  face 5,   5,  7, 11

dodecahedronVertices:
  dw  107,  107,  107
  dw  107,  107, -107
  dw  107, -107,  107
  dw  107, -107, -107
  dw -107,  107,  107
  dw -107,  107, -107
  dw -107, -107,  107
  dw -107, -107, -107
  dw   66,  174,    0
  dw  -66,  174,    0
  dw   66, -174,    0
  dw  -66, -174,    0
  dw  174,    0,   66
  dw  174,    0,  -66
  dw -174,    0,   66
  dw -174,    0,  -66
  dw    0,   66,  174
  dw    0,  -66,  174
  dw    0,   66, -174
  dw    0,  -66, -174
dodecahedronFaces:
  face 1,  13, 12,  0,  8,  1
  face 1,  14, 15,  5,  9,  4
  face 4,  12, 13,  3, 10,  2
  face 2,  15, 14,  6, 11,  7
  face 2,  17, 16,  0, 12,  2
  face 2,  18, 19,  3, 13,  1
  face 4,  16, 17,  6, 14,  4
  face 3,  19, 18,  5, 15,  7
  face 3,   9,  8,  0, 16,  4
  face 3,  10, 11,  6, 17,  2
  face 4,   8,  9,  5, 18,  1
  face 1,  11, 10,  3, 19,  7

shapes:
  shape 8, cubeVertices, 6, cubeFaces
  shape 6, octahedronVertices, 8, octahedronFaces
  shape 4, tetrahedronVertices, 4, tetrahedronFaces
  shape 12, icosahedronVertices, 20, icosahedronFaces
  shape 20, dodecahedronVertices, 12, dodecahedronFaces
endShapes:

currentShape: dw shapes
spanBuffer: dw spanBuffer0
offsetX: dw 0
offsetY: dw 0
xx: dw 0
xy: dw 0
xz: dw 0
yx: dw 0
yy: dw 0
yz: dw 0
zy: dw 0
zz: dw 0
theta: dw 0
dTheta: dw 0
phi: dw 0
dPhi: dw 0
autoRotate: db 1
ya: dw 0       ; bp-0x17
coordAX: dw 0  ; bp-0x15
coordAY: dw 0  ; bp-0x13
coordBX: dw 0  ; bp-0x11
coordBY: dw 0  ; bp-0xf
coordCX: dw 0  ; bp-0xd
coordCY: dw 0  ; bp-0xb
  db 0
yab: dw 0      ; bp-8
yc: dw 0       ; bp-6
yac: dw 0      ; bp-4
colour: dw 0   ; bp-2

spanBufferEntries equ 20
lines equ 200
spanBufferSize equ spanBufferEntries*lines*2

spanBuffer0:
  times spanBufferSize db 0
spanBuffer1:
  times spanBufferSize db 0
spanBufferEnd:

clearSpanBuffer:
  mov cx,lines
  mov bx,spanBufferEntries*2
  add di,2
  mov ax,0xff00
.loop:
  mov word[di-2],di
  mov word[di],ax
  add di,bx
  loop .loop
  ret

sine:
  dw 0x0000, 0x0000, 0x0001, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005
  dw 0x0006, 0x0007, 0x0007, 0x0008, 0x0009, 0x000a, 0x000a, 0x000b
  dw 0x000c, 0x000d, 0x000e, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012
  dw 0x0012, 0x0013, 0x0014, 0x0015, 0x0015, 0x0016, 0x0017, 0x0018
  dw 0x0019, 0x0019, 0x001a, 0x001b, 0x001c, 0x001c, 0x001d, 0x001e
  dw 0x001f, 0x0020, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0024
  dw 0x0025, 0x0026, 0x0027, 0x0027, 0x0028, 0x0029, 0x002a, 0x002a
  dw 0x002b, 0x002c, 0x002d, 0x002e, 0x002e, 0x002f, 0x0030, 0x0031
  dw 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0035, 0x0036, 0x0037
  dw 0x0038, 0x0038, 0x0039, 0x003a, 0x003b, 0x003b, 0x003c, 0x003d
  dw 0x003e, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0042, 0x0043
  dw 0x0044, 0x0045, 0x0045, 0x0046, 0x0047, 0x0048, 0x0048, 0x0049
  dw 0x004a, 0x004b, 0x004b, 0x004c, 0x004d, 0x004e, 0x004e, 0x004f
  dw 0x0050, 0x0051, 0x0051, 0x0052, 0x0053, 0x0054, 0x0054, 0x0055
  dw 0x0056, 0x0056, 0x0057, 0x0058, 0x0059, 0x0059, 0x005a, 0x005b
  dw 0x005c, 0x005c, 0x005d, 0x005e, 0x005f, 0x005f, 0x0060, 0x0061
  dw 0x0061, 0x0062, 0x0063, 0x0064, 0x0064, 0x0065, 0x0066, 0x0067
  dw 0x0067, 0x0068, 0x0069, 0x0069, 0x006a, 0x006b, 0x006c, 0x006c
  dw 0x006d, 0x006e, 0x006e, 0x006f, 0x0070, 0x0070, 0x0071, 0x0072
  dw 0x0073, 0x0073, 0x0074, 0x0075, 0x0075, 0x0076, 0x0077, 0x0077
  dw 0x0078, 0x0079, 0x007a, 0x007a, 0x007b, 0x007c, 0x007c, 0x007d
  dw 0x007e, 0x007e, 0x007f, 0x0080, 0x0080, 0x0081, 0x0082, 0x0082
  dw 0x0083, 0x0084, 0x0084, 0x0085, 0x0086, 0x0086, 0x0087, 0x0088
  dw 0x0088, 0x0089, 0x008a, 0x008a, 0x008b, 0x008c, 0x008c, 0x008d
  dw 0x008e, 0x008e, 0x008f, 0x0090, 0x0090, 0x0091, 0x0092, 0x0092
  dw 0x0093, 0x0094, 0x0094, 0x0095, 0x0095, 0x0096, 0x0097, 0x0097
  dw 0x0098, 0x0099, 0x0099, 0x009a, 0x009b, 0x009b, 0x009c, 0x009c
  dw 0x009d, 0x009e, 0x009e, 0x009f, 0x009f, 0x00a0, 0x00a1, 0x00a1
  dw 0x00a2, 0x00a3, 0x00a3, 0x00a4, 0x00a4, 0x00a5, 0x00a6, 0x00a6
  dw 0x00a7, 0x00a7, 0x00a8, 0x00a8, 0x00a9, 0x00aa, 0x00aa, 0x00ab
  dw 0x00ab, 0x00ac, 0x00ad, 0x00ad, 0x00ae, 0x00ae, 0x00af, 0x00af
  dw 0x00b0, 0x00b1, 0x00b1, 0x00b2, 0x00b2, 0x00b3, 0x00b3, 0x00b4
  dw 0x00b5, 0x00b5, 0x00b6, 0x00b6, 0x00b7, 0x00b7, 0x00b8, 0x00b8
  dw 0x00b9, 0x00b9, 0x00ba, 0x00bb, 0x00bb, 0x00bc, 0x00bc, 0x00bd
  dw 0x00bd, 0x00be, 0x00be, 0x00bf, 0x00bf, 0x00c0, 0x00c0, 0x00c1
  dw 0x00c1, 0x00c2, 0x00c2, 0x00c3, 0x00c3, 0x00c4, 0x00c4, 0x00c5
  dw 0x00c5, 0x00c6, 0x00c6, 0x00c7, 0x00c7, 0x00c8, 0x00c8, 0x00c9
  dw 0x00c9, 0x00ca, 0x00ca, 0x00cb, 0x00cb, 0x00cc, 0x00cc, 0x00cd
  dw 0x00cd, 0x00ce, 0x00ce, 0x00cf, 0x00cf, 0x00cf, 0x00d0, 0x00d0
  dw 0x00d1, 0x00d1, 0x00d2, 0x00d2, 0x00d3, 0x00d3, 0x00d3, 0x00d4
  dw 0x00d4, 0x00d5, 0x00d5, 0x00d6, 0x00d6, 0x00d7, 0x00d7, 0x00d7
  dw 0x00d8, 0x00d8, 0x00d9, 0x00d9, 0x00d9, 0x00da, 0x00da, 0x00db
  dw 0x00db, 0x00db, 0x00dc, 0x00dc, 0x00dd, 0x00dd, 0x00dd, 0x00de
  dw 0x00de, 0x00df, 0x00df, 0x00df, 0x00e0, 0x00e0, 0x00e1, 0x00e1
  dw 0x00e1, 0x00e2, 0x00e2, 0x00e2, 0x00e3, 0x00e3, 0x00e3, 0x00e4
  dw 0x00e4, 0x00e5, 0x00e5, 0x00e5, 0x00e6, 0x00e6, 0x00e6, 0x00e7
  dw 0x00e7, 0x00e7, 0x00e8, 0x00e8, 0x00e8, 0x00e9, 0x00e9, 0x00e9
  dw 0x00ea, 0x00ea, 0x00ea, 0x00ea, 0x00eb, 0x00eb, 0x00eb, 0x00ec
  dw 0x00ec, 0x00ec, 0x00ed, 0x00ed, 0x00ed, 0x00ed, 0x00ee, 0x00ee
  dw 0x00ee, 0x00ef, 0x00ef, 0x00ef, 0x00ef, 0x00f0, 0x00f0, 0x00f0
  dw 0x00f1, 0x00f1, 0x00f1, 0x00f1, 0x00f2, 0x00f2, 0x00f2, 0x00f2
  dw 0x00f3, 0x00f3, 0x00f3, 0x00f3, 0x00f4, 0x00f4, 0x00f4, 0x00f4
  dw 0x00f4, 0x00f5, 0x00f5, 0x00f5, 0x00f5, 0x00f6, 0x00f6, 0x00f6
  dw 0x00f6, 0x00f6, 0x00f7, 0x00f7, 0x00f7, 0x00f7, 0x00f7, 0x00f8
  dw 0x00f8, 0x00f8, 0x00f8, 0x00f8, 0x00f9, 0x00f9, 0x00f9, 0x00f9
  dw 0x00f9, 0x00f9, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa
  dw 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fc
  dw 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fd
  dw 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd
  dw 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe
  dw 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00ff, 0x00ff, 0x00ff, 0x00ff
  dw 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff
  dw 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff
  dw 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff
cosine:
  dw 0x0100, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff
  dw 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff
  dw 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff
  dw 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00fe, 0x00fe, 0x00fe
  dw 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe
  dw 0x00fe, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd
  dw 0x00fd, 0x00fd, 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fc
  dw 0x00fc, 0x00fc, 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fb
  dw 0x00fb, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00f9
  dw 0x00f9, 0x00f9, 0x00f9, 0x00f9, 0x00f9, 0x00f8, 0x00f8, 0x00f8
  dw 0x00f8, 0x00f8, 0x00f7, 0x00f7, 0x00f7, 0x00f7, 0x00f7, 0x00f6
  dw 0x00f6, 0x00f6, 0x00f6, 0x00f6, 0x00f5, 0x00f5, 0x00f5, 0x00f5
  dw 0x00f4, 0x00f4, 0x00f4, 0x00f4, 0x00f4, 0x00f3, 0x00f3, 0x00f3
  dw 0x00f3, 0x00f2, 0x00f2, 0x00f2, 0x00f2, 0x00f1, 0x00f1, 0x00f1
  dw 0x00f1, 0x00f0, 0x00f0, 0x00f0, 0x00ef, 0x00ef, 0x00ef, 0x00ef
  dw 0x00ee, 0x00ee, 0x00ee, 0x00ed, 0x00ed, 0x00ed, 0x00ed, 0x00ec
  dw 0x00ec, 0x00ec, 0x00eb, 0x00eb, 0x00eb, 0x00ea, 0x00ea, 0x00ea
  dw 0x00ea, 0x00e9, 0x00e9, 0x00e9, 0x00e8, 0x00e8, 0x00e8, 0x00e7
  dw 0x00e7, 0x00e7, 0x00e6, 0x00e6, 0x00e6, 0x00e5, 0x00e5, 0x00e5
  dw 0x00e4, 0x00e4, 0x00e3, 0x00e3, 0x00e3, 0x00e2, 0x00e2, 0x00e2
  dw 0x00e1, 0x00e1, 0x00e1, 0x00e0, 0x00e0, 0x00df, 0x00df, 0x00df
  dw 0x00de, 0x00de, 0x00dd, 0x00dd, 0x00dd, 0x00dc, 0x00dc, 0x00db
  dw 0x00db, 0x00db, 0x00da, 0x00da, 0x00d9, 0x00d9, 0x00d9, 0x00d8
  dw 0x00d8, 0x00d7, 0x00d7, 0x00d7, 0x00d6, 0x00d6, 0x00d5, 0x00d5
  dw 0x00d4, 0x00d4, 0x00d3, 0x00d3, 0x00d3, 0x00d2, 0x00d2, 0x00d1
  dw 0x00d1, 0x00d0, 0x00d0, 0x00cf, 0x00cf, 0x00cf, 0x00ce, 0x00ce
  dw 0x00cd, 0x00cd, 0x00cc, 0x00cc, 0x00cb, 0x00cb, 0x00ca, 0x00ca
  dw 0x00c9, 0x00c9, 0x00c8, 0x00c8, 0x00c7, 0x00c7, 0x00c6, 0x00c6
  dw 0x00c5, 0x00c5, 0x00c4, 0x00c4, 0x00c3, 0x00c3, 0x00c2, 0x00c2
  dw 0x00c1, 0x00c1, 0x00c0, 0x00c0, 0x00bf, 0x00bf, 0x00be, 0x00be
  dw 0x00bd, 0x00bd, 0x00bc, 0x00bc, 0x00bb, 0x00bb, 0x00ba, 0x00b9
  dw 0x00b9, 0x00b8, 0x00b8, 0x00b7, 0x00b7, 0x00b6, 0x00b6, 0x00b5
  dw 0x00b5, 0x00b4, 0x00b3, 0x00b3, 0x00b2, 0x00b2, 0x00b1, 0x00b1
  dw 0x00b0, 0x00af, 0x00af, 0x00ae, 0x00ae, 0x00ad, 0x00ad, 0x00ac
  dw 0x00ab, 0x00ab, 0x00aa, 0x00aa, 0x00a9, 0x00a8, 0x00a8, 0x00a7
  dw 0x00a7, 0x00a6, 0x00a6, 0x00a5, 0x00a4, 0x00a4, 0x00a3, 0x00a3
  dw 0x00a2, 0x00a1, 0x00a1, 0x00a0, 0x009f, 0x009f, 0x009e, 0x009e
  dw 0x009d, 0x009c, 0x009c, 0x009b, 0x009b, 0x009a, 0x0099, 0x0099
  dw 0x0098, 0x0097, 0x0097, 0x0096, 0x0095, 0x0095, 0x0094, 0x0094
  dw 0x0093, 0x0092, 0x0092, 0x0091, 0x0090, 0x0090, 0x008f, 0x008e
  dw 0x008e, 0x008d, 0x008c, 0x008c, 0x008b, 0x008a, 0x008a, 0x0089
  dw 0x0088, 0x0088, 0x0087, 0x0086, 0x0086, 0x0085, 0x0084, 0x0084
  dw 0x0083, 0x0082, 0x0082, 0x0081, 0x0080, 0x0080, 0x007f, 0x007e
  dw 0x007e, 0x007d, 0x007c, 0x007c, 0x007b, 0x007a, 0x007a, 0x0079
  dw 0x0078, 0x0077, 0x0077, 0x0076, 0x0075, 0x0075, 0x0074, 0x0073
  dw 0x0073, 0x0072, 0x0071, 0x0070, 0x0070, 0x006f, 0x006e, 0x006e
  dw 0x006d, 0x006c, 0x006c, 0x006b, 0x006a, 0x0069, 0x0069, 0x0068
  dw 0x0067, 0x0067, 0x0066, 0x0065, 0x0064, 0x0064, 0x0063, 0x0062
  dw 0x0061, 0x0061, 0x0060, 0x005f, 0x005f, 0x005e, 0x005d, 0x005c
  dw 0x005c, 0x005b, 0x005a, 0x0059, 0x0059, 0x0058, 0x0057, 0x0056
  dw 0x0056, 0x0055, 0x0054, 0x0054, 0x0053, 0x0052, 0x0051, 0x0051
  dw 0x0050, 0x004f, 0x004e, 0x004e, 0x004d, 0x004c, 0x004b, 0x004b
  dw 0x004a, 0x0049, 0x0048, 0x0048, 0x0047, 0x0046, 0x0045, 0x0045
  dw 0x0044, 0x0043, 0x0042, 0x0042, 0x0041, 0x0040, 0x003f, 0x003e
  dw 0x003e, 0x003d, 0x003c, 0x003b, 0x003b, 0x003a, 0x0039, 0x0038
  dw 0x0038, 0x0037, 0x0036, 0x0035, 0x0035, 0x0034, 0x0033, 0x0032
  dw 0x0031, 0x0031, 0x0030, 0x002f, 0x002e, 0x002e, 0x002d, 0x002c
  dw 0x002b, 0x002a, 0x002a, 0x0029, 0x0028, 0x0027, 0x0027, 0x0026
  dw 0x0025, 0x0024, 0x0024, 0x0023, 0x0022, 0x0021, 0x0020, 0x0020
  dw 0x001f, 0x001e, 0x001d, 0x001c, 0x001c, 0x001b, 0x001a, 0x0019
  dw 0x0019, 0x0018, 0x0017, 0x0016, 0x0015, 0x0015, 0x0014, 0x0013
  dw 0x0012, 0x0012, 0x0011, 0x0010, 0x000f, 0x000e, 0x000e, 0x000d
  dw 0x000c, 0x000b, 0x000a, 0x000a, 0x0009, 0x0008, 0x0007, 0x0007
  dw 0x0006, 0x0005, 0x0004, 0x0003, 0x0003, 0x0002, 0x0001, 0x0000
  dw 0x0000, 0x0000, 0xffff, 0xfffe, 0xfffd, 0xfffd, 0xfffc, 0xfffb
  dw 0xfffa, 0xfff9, 0xfff9, 0xfff8, 0xfff7, 0xfff6, 0xfff6, 0xfff5
  dw 0xfff4, 0xfff3, 0xfff2, 0xfff2, 0xfff1, 0xfff0, 0xffef, 0xffee
  dw 0xffee, 0xffed, 0xffec, 0xffeb, 0xffeb, 0xffea, 0xffe9, 0xffe8
  dw 0xffe7, 0xffe7, 0xffe6, 0xffe5, 0xffe4, 0xffe4, 0xffe3, 0xffe2
  dw 0xffe1, 0xffe0, 0xffe0, 0xffdf, 0xffde, 0xffdd, 0xffdc, 0xffdc
  dw 0xffdb, 0xffda, 0xffd9, 0xffd9, 0xffd8, 0xffd7, 0xffd6, 0xffd6
  dw 0xffd5, 0xffd4, 0xffd3, 0xffd2, 0xffd2, 0xffd1, 0xffd0, 0xffcf
  dw 0xffcf, 0xffce, 0xffcd, 0xffcc, 0xffcb, 0xffcb, 0xffca, 0xffc9
  dw 0xffc8, 0xffc8, 0xffc7, 0xffc6, 0xffc5, 0xffc5, 0xffc4, 0xffc3
  dw 0xffc2, 0xffc2, 0xffc1, 0xffc0, 0xffbf, 0xffbe, 0xffbe, 0xffbd
  dw 0xffbc, 0xffbb, 0xffbb, 0xffba, 0xffb9, 0xffb8, 0xffb8, 0xffb7
  dw 0xffb6, 0xffb5, 0xffb5, 0xffb4, 0xffb3, 0xffb2, 0xffb2, 0xffb1
  dw 0xffb0, 0xffaf, 0xffaf, 0xffae, 0xffad, 0xffac, 0xffac, 0xffab
  dw 0xffaa, 0xffaa, 0xffa9, 0xffa8, 0xffa7, 0xffa7, 0xffa6, 0xffa5
  dw 0xffa4, 0xffa4, 0xffa3, 0xffa2, 0xffa1, 0xffa1, 0xffa0, 0xff9f
  dw 0xff9f, 0xff9e, 0xff9d, 0xff9c, 0xff9c, 0xff9b, 0xff9a, 0xff99
  dw 0xff99, 0xff98, 0xff97, 0xff97, 0xff96, 0xff95, 0xff94, 0xff94
  dw 0xff93, 0xff92, 0xff92, 0xff91, 0xff90, 0xff90, 0xff8f, 0xff8e
  dw 0xff8d, 0xff8d, 0xff8c, 0xff8b, 0xff8b, 0xff8a, 0xff89, 0xff89
  dw 0xff88, 0xff87, 0xff86, 0xff86, 0xff85, 0xff84, 0xff84, 0xff83
  dw 0xff82, 0xff82, 0xff81, 0xff80, 0xff80, 0xff7f, 0xff7e, 0xff7e
  dw 0xff7d, 0xff7c, 0xff7c, 0xff7b, 0xff7a, 0xff7a, 0xff79, 0xff78
  dw 0xff78, 0xff77, 0xff76, 0xff76, 0xff75, 0xff74, 0xff74, 0xff73
  dw 0xff72, 0xff72, 0xff71, 0xff70, 0xff70, 0xff6f, 0xff6e, 0xff6e
  dw 0xff6d, 0xff6c, 0xff6c, 0xff6b, 0xff6b, 0xff6a, 0xff69, 0xff69
  dw 0xff68, 0xff67, 0xff67, 0xff66, 0xff65, 0xff65, 0xff64, 0xff64
  dw 0xff63, 0xff62, 0xff62, 0xff61, 0xff61, 0xff60, 0xff5f, 0xff5f
  dw 0xff5e, 0xff5d, 0xff5d, 0xff5c, 0xff5c, 0xff5b, 0xff5a, 0xff5a
  dw 0xff59, 0xff59, 0xff58, 0xff58, 0xff57, 0xff56, 0xff56, 0xff55
  dw 0xff55, 0xff54, 0xff53, 0xff53, 0xff52, 0xff52, 0xff51, 0xff51
  dw 0xff50, 0xff4f, 0xff4f, 0xff4e, 0xff4e, 0xff4d, 0xff4d, 0xff4c
  dw 0xff4b, 0xff4b, 0xff4a, 0xff4a, 0xff49, 0xff49, 0xff48, 0xff48
  dw 0xff47, 0xff47, 0xff46, 0xff45, 0xff45, 0xff44, 0xff44, 0xff43
  dw 0xff43, 0xff42, 0xff42, 0xff41, 0xff41, 0xff40, 0xff40, 0xff3f
  dw 0xff3f, 0xff3e, 0xff3e, 0xff3d, 0xff3d, 0xff3c, 0xff3c, 0xff3b
  dw 0xff3b, 0xff3a, 0xff3a, 0xff39, 0xff39, 0xff38, 0xff38, 0xff37
  dw 0xff37, 0xff36, 0xff36, 0xff35, 0xff35, 0xff34, 0xff34, 0xff33
  dw 0xff33, 0xff32, 0xff32, 0xff31, 0xff31, 0xff31, 0xff30, 0xff30
  dw 0xff2f, 0xff2f, 0xff2e, 0xff2e, 0xff2d, 0xff2d, 0xff2d, 0xff2c
  dw 0xff2c, 0xff2b, 0xff2b, 0xff2a, 0xff2a, 0xff29, 0xff29, 0xff29
  dw 0xff28, 0xff28, 0xff27, 0xff27, 0xff27, 0xff26, 0xff26, 0xff25
  dw 0xff25, 0xff25, 0xff24, 0xff24, 0xff23, 0xff23, 0xff23, 0xff22
  dw 0xff22, 0xff21, 0xff21, 0xff21, 0xff20, 0xff20, 0xff1f, 0xff1f
  dw 0xff1f, 0xff1e, 0xff1e, 0xff1e, 0xff1d, 0xff1d, 0xff1d, 0xff1c
  dw 0xff1c, 0xff1b, 0xff1b, 0xff1b, 0xff1a, 0xff1a, 0xff1a, 0xff19
  dw 0xff19, 0xff19, 0xff18, 0xff18, 0xff18, 0xff17, 0xff17, 0xff17
  dw 0xff16, 0xff16, 0xff16, 0xff16, 0xff15, 0xff15, 0xff15, 0xff14
  dw 0xff14, 0xff14, 0xff13, 0xff13, 0xff13, 0xff13, 0xff12, 0xff12
  dw 0xff12, 0xff11, 0xff11, 0xff11, 0xff11, 0xff10, 0xff10, 0xff10
  dw 0xff0f, 0xff0f, 0xff0f, 0xff0f, 0xff0e, 0xff0e, 0xff0e, 0xff0e
  dw 0xff0d, 0xff0d, 0xff0d, 0xff0d, 0xff0c, 0xff0c, 0xff0c, 0xff0c
  dw 0xff0c, 0xff0b, 0xff0b, 0xff0b, 0xff0b, 0xff0a, 0xff0a, 0xff0a
  dw 0xff0a, 0xff0a, 0xff09, 0xff09, 0xff09, 0xff09, 0xff09, 0xff08
  dw 0xff08, 0xff08, 0xff08, 0xff08, 0xff07, 0xff07, 0xff07, 0xff07
  dw 0xff07, 0xff07, 0xff06, 0xff06, 0xff06, 0xff06, 0xff06, 0xff06
  dw 0xff05, 0xff05, 0xff05, 0xff05, 0xff05, 0xff05, 0xff05, 0xff04
  dw 0xff04, 0xff04, 0xff04, 0xff04, 0xff04, 0xff04, 0xff04, 0xff03
  dw 0xff03, 0xff03, 0xff03, 0xff03, 0xff03, 0xff03, 0xff03, 0xff03
  dw 0xff02, 0xff02, 0xff02, 0xff02, 0xff02, 0xff02, 0xff02, 0xff02
  dw 0xff02, 0xff02, 0xff02, 0xff02, 0xff01, 0xff01, 0xff01, 0xff01
  dw 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01
  dw 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01
  dw 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01
  dw 0xff00, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01
  dw 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01
  dw 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff01
  dw 0xff01, 0xff01, 0xff01, 0xff01, 0xff01, 0xff02, 0xff02, 0xff02
  dw 0xff02, 0xff02, 0xff02, 0xff02, 0xff02, 0xff02, 0xff02, 0xff02
  dw 0xff02, 0xff03, 0xff03, 0xff03, 0xff03, 0xff03, 0xff03, 0xff03
  dw 0xff03, 0xff03, 0xff04, 0xff04, 0xff04, 0xff04, 0xff04, 0xff04
  dw 0xff04, 0xff04, 0xff05, 0xff05, 0xff05, 0xff05, 0xff05, 0xff05
  dw 0xff05, 0xff06, 0xff06, 0xff06, 0xff06, 0xff06, 0xff06, 0xff07
  dw 0xff07, 0xff07, 0xff07, 0xff07, 0xff07, 0xff08, 0xff08, 0xff08
  dw 0xff08, 0xff08, 0xff09, 0xff09, 0xff09, 0xff09, 0xff09, 0xff0a
  dw 0xff0a, 0xff0a, 0xff0a, 0xff0a, 0xff0b, 0xff0b, 0xff0b, 0xff0b
  dw 0xff0c, 0xff0c, 0xff0c, 0xff0c, 0xff0c, 0xff0d, 0xff0d, 0xff0d
  dw 0xff0d, 0xff0e, 0xff0e, 0xff0e, 0xff0e, 0xff0f, 0xff0f, 0xff0f
  dw 0xff0f, 0xff10, 0xff10, 0xff10, 0xff11, 0xff11, 0xff11, 0xff11
  dw 0xff12, 0xff12, 0xff12, 0xff13, 0xff13, 0xff13, 0xff13, 0xff14
  dw 0xff14, 0xff14, 0xff15, 0xff15, 0xff15, 0xff16, 0xff16, 0xff16
  dw 0xff16, 0xff17, 0xff17, 0xff17, 0xff18, 0xff18, 0xff18, 0xff19
  dw 0xff19, 0xff19, 0xff1a, 0xff1a, 0xff1a, 0xff1b, 0xff1b, 0xff1b
  dw 0xff1c, 0xff1c, 0xff1d, 0xff1d, 0xff1d, 0xff1e, 0xff1e, 0xff1e
  dw 0xff1f, 0xff1f, 0xff1f, 0xff20, 0xff20, 0xff21, 0xff21, 0xff21
  dw 0xff22, 0xff22, 0xff23, 0xff23, 0xff23, 0xff24, 0xff24, 0xff25
  dw 0xff25, 0xff25, 0xff26, 0xff26, 0xff27, 0xff27, 0xff27, 0xff28
  dw 0xff28, 0xff29, 0xff29, 0xff29, 0xff2a, 0xff2a, 0xff2b, 0xff2b
  dw 0xff2c, 0xff2c, 0xff2d, 0xff2d, 0xff2d, 0xff2e, 0xff2e, 0xff2f
  dw 0xff2f, 0xff30, 0xff30, 0xff31, 0xff31, 0xff31, 0xff32, 0xff32
  dw 0xff33, 0xff33, 0xff34, 0xff34, 0xff35, 0xff35, 0xff36, 0xff36
  dw 0xff37, 0xff37, 0xff38, 0xff38, 0xff39, 0xff39, 0xff3a, 0xff3a
  dw 0xff3b, 0xff3b, 0xff3c, 0xff3c, 0xff3d, 0xff3d, 0xff3e, 0xff3e
  dw 0xff3f, 0xff3f, 0xff40, 0xff40, 0xff41, 0xff41, 0xff42, 0xff42
  dw 0xff43, 0xff43, 0xff44, 0xff44, 0xff45, 0xff45, 0xff46, 0xff47
  dw 0xff47, 0xff48, 0xff48, 0xff49, 0xff49, 0xff4a, 0xff4a, 0xff4b
  dw 0xff4b, 0xff4c, 0xff4d, 0xff4d, 0xff4e, 0xff4e, 0xff4f, 0xff4f
  dw 0xff50, 0xff51, 0xff51, 0xff52, 0xff52, 0xff53, 0xff53, 0xff54
  dw 0xff55, 0xff55, 0xff56, 0xff56, 0xff57, 0xff58, 0xff58, 0xff59
  dw 0xff59, 0xff5a, 0xff5a, 0xff5b, 0xff5c, 0xff5c, 0xff5d, 0xff5d
  dw 0xff5e, 0xff5f, 0xff5f, 0xff60, 0xff61, 0xff61, 0xff62, 0xff62
  dw 0xff63, 0xff64, 0xff64, 0xff65, 0xff65, 0xff66, 0xff67, 0xff67
  dw 0xff68, 0xff69, 0xff69, 0xff6a, 0xff6b, 0xff6b, 0xff6c, 0xff6c
  dw 0xff6d, 0xff6e, 0xff6e, 0xff6f, 0xff70, 0xff70, 0xff71, 0xff72
  dw 0xff72, 0xff73, 0xff74, 0xff74, 0xff75, 0xff76, 0xff76, 0xff77
  dw 0xff78, 0xff78, 0xff79, 0xff7a, 0xff7a, 0xff7b, 0xff7c, 0xff7c
  dw 0xff7d, 0xff7e, 0xff7e, 0xff7f, 0xff80, 0xff80, 0xff81, 0xff82
  dw 0xff82, 0xff83, 0xff84, 0xff84, 0xff85, 0xff86, 0xff86, 0xff87
  dw 0xff88, 0xff89, 0xff89, 0xff8a, 0xff8b, 0xff8b, 0xff8c, 0xff8d
  dw 0xff8d, 0xff8e, 0xff8f, 0xff90, 0xff90, 0xff91, 0xff92, 0xff92
  dw 0xff93, 0xff94, 0xff94, 0xff95, 0xff96, 0xff97, 0xff97, 0xff98
  dw 0xff99, 0xff99, 0xff9a, 0xff9b, 0xff9c, 0xff9c, 0xff9d, 0xff9e
  dw 0xff9f, 0xff9f, 0xffa0, 0xffa1, 0xffa1, 0xffa2, 0xffa3, 0xffa4
  dw 0xffa4, 0xffa5, 0xffa6, 0xffa7, 0xffa7, 0xffa8, 0xffa9, 0xffaa
  dw 0xffaa, 0xffab, 0xffac, 0xffac, 0xffad, 0xffae, 0xffaf, 0xffaf
  dw 0xffb0, 0xffb1, 0xffb2, 0xffb2, 0xffb3, 0xffb4, 0xffb5, 0xffb5
  dw 0xffb6, 0xffb7, 0xffb8, 0xffb8, 0xffb9, 0xffba, 0xffbb, 0xffbb
  dw 0xffbc, 0xffbd, 0xffbe, 0xffbe, 0xffbf, 0xffc0, 0xffc1, 0xffc2
  dw 0xffc2, 0xffc3, 0xffc4, 0xffc5, 0xffc5, 0xffc6, 0xffc7, 0xffc8
  dw 0xffc8, 0xffc9, 0xffca, 0xffcb, 0xffcb, 0xffcc, 0xffcd, 0xffce
  dw 0xffcf, 0xffcf, 0xffd0, 0xffd1, 0xffd2, 0xffd2, 0xffd3, 0xffd4
  dw 0xffd5, 0xffd6, 0xffd6, 0xffd7, 0xffd8, 0xffd9, 0xffd9, 0xffda
  dw 0xffdb, 0xffdc, 0xffdc, 0xffdd, 0xffde, 0xffdf, 0xffe0, 0xffe0
  dw 0xffe1, 0xffe2, 0xffe3, 0xffe4, 0xffe4, 0xffe5, 0xffe6, 0xffe7
  dw 0xffe7, 0xffe8, 0xffe9, 0xffea, 0xffeb, 0xffeb, 0xffec, 0xffed
  dw 0xffee, 0xffee, 0xffef, 0xfff0, 0xfff1, 0xfff2, 0xfff2, 0xfff3
  dw 0xfff4, 0xfff5, 0xfff6, 0xfff6, 0xfff7, 0xfff8, 0xfff9, 0xfff9
  dw 0xfffa, 0xfffb, 0xfffc, 0xfffd, 0xfffd, 0xfffe, 0xffff, 0x0000
  dw 0x0000, 0x0000, 0x0001, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005
  dw 0x0006, 0x0007, 0x0007, 0x0008, 0x0009, 0x000a, 0x000a, 0x000b
  dw 0x000c, 0x000d, 0x000e, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012
  dw 0x0012, 0x0013, 0x0014, 0x0015, 0x0015, 0x0016, 0x0017, 0x0018
  dw 0x0019, 0x0019, 0x001a, 0x001b, 0x001c, 0x001c, 0x001d, 0x001e
  dw 0x001f, 0x0020, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0024
  dw 0x0025, 0x0026, 0x0027, 0x0027, 0x0028, 0x0029, 0x002a, 0x002a
  dw 0x002b, 0x002c, 0x002d, 0x002e, 0x002e, 0x002f, 0x0030, 0x0031
  dw 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0035, 0x0036, 0x0037
  dw 0x0038, 0x0038, 0x0039, 0x003a, 0x003b, 0x003b, 0x003c, 0x003d
  dw 0x003e, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0042, 0x0043
  dw 0x0044, 0x0045, 0x0045, 0x0046, 0x0047, 0x0048, 0x0048, 0x0049
  dw 0x004a, 0x004b, 0x004b, 0x004c, 0x004d, 0x004e, 0x004e, 0x004f
  dw 0x0050, 0x0051, 0x0051, 0x0052, 0x0053, 0x0054, 0x0054, 0x0055
  dw 0x0056, 0x0056, 0x0057, 0x0058, 0x0059, 0x0059, 0x005a, 0x005b
  dw 0x005c, 0x005c, 0x005d, 0x005e, 0x005f, 0x005f, 0x0060, 0x0061
  dw 0x0061, 0x0062, 0x0063, 0x0064, 0x0064, 0x0065, 0x0066, 0x0067
  dw 0x0067, 0x0068, 0x0069, 0x0069, 0x006a, 0x006b, 0x006c, 0x006c
  dw 0x006d, 0x006e, 0x006e, 0x006f, 0x0070, 0x0070, 0x0071, 0x0072
  dw 0x0073, 0x0073, 0x0074, 0x0075, 0x0075, 0x0076, 0x0077, 0x0077
  dw 0x0078, 0x0079, 0x007a, 0x007a, 0x007b, 0x007c, 0x007c, 0x007d
  dw 0x007e, 0x007e, 0x007f, 0x0080, 0x0080, 0x0081, 0x0082, 0x0082
  dw 0x0083, 0x0084, 0x0084, 0x0085, 0x0086, 0x0086, 0x0087, 0x0088
  dw 0x0088, 0x0089, 0x008a, 0x008a, 0x008b, 0x008c, 0x008c, 0x008d
  dw 0x008e, 0x008e, 0x008f, 0x0090, 0x0090, 0x0091, 0x0092, 0x0092
  dw 0x0093, 0x0094, 0x0094, 0x0095, 0x0095, 0x0096, 0x0097, 0x0097
  dw 0x0098, 0x0099, 0x0099, 0x009a, 0x009b, 0x009b, 0x009c, 0x009c
  dw 0x009d, 0x009e, 0x009e, 0x009f, 0x009f, 0x00a0, 0x00a1, 0x00a1
  dw 0x00a2, 0x00a3, 0x00a3, 0x00a4, 0x00a4, 0x00a5, 0x00a6, 0x00a6
  dw 0x00a7, 0x00a7, 0x00a8, 0x00a8, 0x00a9, 0x00aa, 0x00aa, 0x00ab
  dw 0x00ab, 0x00ac, 0x00ad, 0x00ad, 0x00ae, 0x00ae, 0x00af, 0x00af
  dw 0x00b0, 0x00b1, 0x00b1, 0x00b2, 0x00b2, 0x00b3, 0x00b3, 0x00b4
  dw 0x00b5, 0x00b5, 0x00b6, 0x00b6, 0x00b7, 0x00b7, 0x00b8, 0x00b8
  dw 0x00b9, 0x00b9, 0x00ba, 0x00bb, 0x00bb, 0x00bc, 0x00bc, 0x00bd
  dw 0x00bd, 0x00be, 0x00be, 0x00bf, 0x00bf, 0x00c0, 0x00c0, 0x00c1
  dw 0x00c1, 0x00c2, 0x00c2, 0x00c3, 0x00c3, 0x00c4, 0x00c4, 0x00c5
  dw 0x00c5, 0x00c6, 0x00c6, 0x00c7, 0x00c7, 0x00c8, 0x00c8, 0x00c9
  dw 0x00c9, 0x00ca, 0x00ca, 0x00cb, 0x00cb, 0x00cc, 0x00cc, 0x00cd
  dw 0x00cd, 0x00ce, 0x00ce, 0x00cf, 0x00cf, 0x00cf, 0x00d0, 0x00d0
  dw 0x00d1, 0x00d1, 0x00d2, 0x00d2, 0x00d3, 0x00d3, 0x00d3, 0x00d4
  dw 0x00d4, 0x00d5, 0x00d5, 0x00d6, 0x00d6, 0x00d7, 0x00d7, 0x00d7
  dw 0x00d8, 0x00d8, 0x00d9, 0x00d9, 0x00d9, 0x00da, 0x00da, 0x00db
  dw 0x00db, 0x00db, 0x00dc, 0x00dc, 0x00dd, 0x00dd, 0x00dd, 0x00de
  dw 0x00de, 0x00df, 0x00df, 0x00df, 0x00e0, 0x00e0, 0x00e1, 0x00e1
  dw 0x00e1, 0x00e2, 0x00e2, 0x00e2, 0x00e3, 0x00e3, 0x00e3, 0x00e4
  dw 0x00e4, 0x00e5, 0x00e5, 0x00e5, 0x00e6, 0x00e6, 0x00e6, 0x00e7
  dw 0x00e7, 0x00e7, 0x00e8, 0x00e8, 0x00e8, 0x00e9, 0x00e9, 0x00e9
  dw 0x00ea, 0x00ea, 0x00ea, 0x00ea, 0x00eb, 0x00eb, 0x00eb, 0x00ec
  dw 0x00ec, 0x00ec, 0x00ed, 0x00ed, 0x00ed, 0x00ed, 0x00ee, 0x00ee
  dw 0x00ee, 0x00ef, 0x00ef, 0x00ef, 0x00ef, 0x00f0, 0x00f0, 0x00f0
  dw 0x00f1, 0x00f1, 0x00f1, 0x00f1, 0x00f2, 0x00f2, 0x00f2, 0x00f2
  dw 0x00f3, 0x00f3, 0x00f3, 0x00f3, 0x00f4, 0x00f4, 0x00f4, 0x00f4
  dw 0x00f4, 0x00f5, 0x00f5, 0x00f5, 0x00f5, 0x00f6, 0x00f6, 0x00f6
  dw 0x00f6, 0x00f6, 0x00f7, 0x00f7, 0x00f7, 0x00f7, 0x00f7, 0x00f8
  dw 0x00f8, 0x00f8, 0x00f8, 0x00f8, 0x00f9, 0x00f9, 0x00f9, 0x00f9
  dw 0x00f9, 0x00f9, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa, 0x00fa
  dw 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fb, 0x00fc
  dw 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fc, 0x00fd
  dw 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd, 0x00fd
  dw 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00fe
  dw 0x00fe, 0x00fe, 0x00fe, 0x00fe, 0x00ff, 0x00ff, 0x00ff, 0x00ff
  dw 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff
  dw 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff
  dw 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff


;0000 1111 2222 3333
;0101 0202 0303 1212 1313 2323
;0001 0111 0002 0222 0003 0333 1112 1222 1113 1333 2223 2333
;0012 0013 0023 1123 0112 0113 0223 1223 0122 0133 0233 1233
;0123


