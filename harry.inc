;--------------------------------
;Constants
;--------------------------------

SCRGRAPH      equ $d0 ; Clear4 color, $f0 = clear4 mono
SCRTEXT       equ $d0
TOPSCR        equ $8100
SCR_W         equ 20
SCR_H         equ 15
    
green         equ $00
yellow        equ $40
blue          equ $80
red           equ $c0

;--------------------------------
;Zeropage addresses
;--------------------------------

TMP           equ $60
ADDR          equ $80
SPR           equ $82
TILE          equ $84
LEV           equ $86
SPRADR        equ $88

;--------------------------------
;Game variables ($240-$3bf is free in assembler)
;--------------------------------

COLTIME       equ $240
COLCOUNT      equ $241
KEY           equ $242
;JOY           equ $243
SCORE         equ $244 ; 4 bytes
NUM           equ $248 ; 4 bytes
LIVES         equ $24c
LEVN          equ $24d
LUNCH         equ $24e
SPRTMP        equ $24f ; 4 bytes
TIME          equ $253
PX            equ $254
OPX           equ $255
PDY           equ $256
PF            equ $257
OPF           equ $258
PDX           equ $259
PJ            equ $25a
PDIR          equ $25b
OPDIR         equ $25c
PLAD          equ $25d
OPLAD         equ $25e
PFALL         equ $25f
DEAD          equ $260
LADSOLID      equ $261
PY            equ $262
OPY           equ $263
CONVMV        equ $26
BKFRAME       equ $265
BKTIME        equ $266
HAYLPTIME     equ $267
BARRELT       equ $268
BARRELX       equ $269
TX            equ $26a
TY            equ $26b
BARRELY       equ $26c
BARRELDX      equ $26d
BARRELDY      equ $26e
BARRELD       equ $26f
BARRELE       equ $270

;--------------------------------
;System calls
;--------------------------------

OSRDKEY       equ $fe71
OSRDCH        equ $fe94
WAITSYNC      equ $fe66

;--------------------------------
;Memory declaration
;--------------------------------

hiname1       equ $300 ;(10)
hiname2       equ $30A ;(10)
hiname3       equ $314 ;(10)
hibyte1       equ $320 ;(10)
hibyte2       equ $32A ;(10)
hibyte3       equ $334 ;(10)
hibyte4       equ $340 ;(10)

level         equ $400 ;(300) 20 x 15 bytes
SPRX          equ $670 ;(16) X (255=off)
SPRY          equ $680 ;(16) Y
SPRF          equ $690 ;(16) Frame
SPRT          equ $6A0 ;(16) Type (even numbers)
SPRD          equ $6B0 ;(16) Direction
SPRD1         equ $6C0 ;(16) Data 1
SPRD2         equ $6D0 ;(16) Data 2
SPRD3         equ $6E0 ;(16) Data 2
SPR1          equ $6F0 ;(16) First run (255=first run, 0=not first run) 

;--------------------------------
;Init highscore list
;--------------------------------
;	Fill list with:
;	  1) HHH 00100000
;	  2) HHH 00090000
;	         ...
;	 10) HHH 00010000
;--------------------------------

inithiscore:
        ldx #9
        ldy #1
rl1:
        lda #'H'
        sta hiname1,x
        sta hiname2,x
        sta hiname3,x
        lda #0
        sta hibyte1,x
        sta hibyte2,x
        sta hibyte4,x
        tya
        sta hibyte3,x
        iny
        dex
        bpl rl1
        
        lda #$10
        sta hibyte3

;--------------------------------
;Game entry main loop
;--------------------------------

start:
        lda #6
        sta COLTIME

	lda #SCRTEXT   ; Init screenmode
        jsr setmode
                
        jsr title      ; Print header

        ldx #<rs       ; Print footer
        ldy #>rs
        jsr PUTSTRING        
        ldx #<prsspc
        ldy #>prsspc
        jsr PUTSTRING
st1:
	ldx #5         ; Wait for SPACE
	jsr getkey
        bne st1

        lda KEY
        jsr startgame  ; Play game
        
        jsr findhi     ; Check for new highscore
        cpx #10
        beq start
        
        stx TMP+7
        jsr movehi     ; Make room for new entry
        ldx TMP+7
        
        lda SCORE      ; Fill in new highscore
        sta hibyte1,x
        lda SCORE+1
        sta hibyte2,x
        lda SCORE+2
        sta hibyte3,x
        lda SCORE+3
        sta hibyte4,x
        lda #32
        sta hiname1,x  ; Fill in new name with 3 spaces
        sta hiname2,x
        sta hiname3,x
        
	lda #SCRTEXT   ; Clear screen
        jsr setmode
       
        jsr title      ; Print header

	ldx #<typename ; Print congratulations text
	ldy #>typename
	jsr PUTSTRING

	lda #(red/4+red/16+red/64+red)
	sta COLOR
st2:
        jsr getchr     ; Wait for keypress
	beq st1        ; If DEL start again

	ldx #12        ; Print and store 1st letter of new name
	pha
	lda TMP+7
	adc #6
	tay
	pla
	jsr PUTCHAR
	ldx TMP+7
        sta hiname1,x       
st3:
        jsr getchr     ; Print and store 2nd letter of new name
        beq st2        ; If DEL go back to 1st letter
	ldx #13
	pha
	lda TMP+7
	adc #6
	tay
	pla
	jsr PUTCHAR
	ldx TMP+7
        sta hiname2,x
                
        jsr getchr     ; Print and store 3rd letter of new name
        beq st3        ; If DEL go back to 2nd letter
	ldx #14
	pha
	lda TMP+7
	adc #6
	tay
	pla
	jsr PUTCHAR
	ldx TMP+7
        sta hiname3,x                     
        
        jmp start      ; Start again

;--------------------------------
;Print title screen
;--------------------------------
;	Print game title
;	Print high scores text
;	Print high scorelist
;--------------------------------

title:
        ldx #<banner  ; Print game title
        ldy #>banner
        jsr PUTSTRING
        
        ldx #<hiscr   ; Print high score text
        ldy #>hiscr
        jsr PUTSTRING

        jsr hiscores  ; Print high scorelist
        
        rts

;--------------------------------
;Print score
;--------------------------------
;	Print score text + number on screen
;--------------------------------

printscr:
        ldy #0
	ldx #3
prs1:
        lda SCORE,x    ; Fill in scorenumber
	jsr getnum1
	sta prtnum+2,y
	iny	
        lda SCORE,x
        and #15
        clc
        adc #48
	sta prtnum+2,y
	iny
	dex
	bpl prs1

	ldx #<prtscore ; Print score text + number
	ldy #>prtscore
	jsr PUTSTRING

	rts

;--------------------------------
;Start the game
;--------------------------------
;	Play until dead
;--------------------------------

startgame:
        lda #3         ; Init gamevars
        sta LIVES
        
        lda #0
        sta SCORE
        sta SCORE+1
        sta SCORE+2
        sta SCORE+3
        
	lda #0
	sta LEVN              
gamelp:
        lda #SCRTEXT   ; Init screenmode
        jsr setmode

        ldx #<levelstr ; Print LEVEL in middle of screen  
        ldy #>levelstr
        jsr PUTSTRING
        
        lda #18       ; Print levelnumber
	sta XPOS
	lda #12
	sta YPOS
	jsr levnum

        jsr delay     ; Wait 2 sec

        lda #SCRGRAPH  ; Init screenmode
        jsr setmode

        jsr clrscr    ; Clear playfield
        jsr loadlevel ; Decode level into tmp levelbuffer
        jsr drawlevel ; Draw level with tiles

        jsr initsprites ; Initialize sprite data ??????

        jsr printscr  ; Print scoreline
             
        ldx #<livesstr ; Print lives
        ldy #>livesstr
        jsr PUTSTRING
        lda LIVES
        clc
        adc #48
	ldx #31
	ldy #0
	jsr PUTCHAR

        lda #3        ; Print timer
        sta TIME
        lda #'9'
        sta timenum+2
        sta timenum+3
        sta timenum+4
        lda LEVN
        lsr a
        lsr a
        lsr a
        lsr a
        beq sg2
        cmp #6
        bcc sg1
        lda #6
sg1:
        sta TMP        
        lda #9+48
        sec
        sbc TMP
        sta timenum+2
sg2:
        ldx #<timestr
        ldy #>timestr
        jsr PUTSTRING

        ldx #<level2str ; Print levelnumber
        ldy #>level2str
        jsr PUTSTRING
	lda #6
	sta XPOS
	lda #23
	sta YPOS
        jsr levnum

        lda #128     ; Init game level vars
        sta PY
        sta OPY

        lda #0
        sta PX
        sta OPX
        sta PDY
        sta PF
        sta OPF
        sta PDX
        sta PJ
        sta PDIR
        sta OPDIR
        sta PLAD
        sta OPLAD
        sta PFALL
        sta DEAD
        sta LADSOLID
        
        ldx #0      ; Draw Harry facing right
        ldy #128-24     ;!!!!!!!!!!!!!!!!!!!
        sta CONVMV
        sta BKFRAME
        jsr drawsprite

        lda #16     ; Init game level vars
        sta BKTIME
        lda #50
        sta HAYLPTIME
        lda #255
        sta BARRELX
        lsr a
        sta BARRELT
mainl:
;	jsr WAITSYNC ; Wait for vert sync

	lda LEVN
	cmp #7
	bne nowait
	jsr WAITSYNC
nowait:

        
        lda OPF
        lsr a
        lsr a
        and #3
        tax
        lda frames,x
        clc
        ldx OPLAD
        bne sg3
                
        adc OPDIR
        pha
        jmp sg4
sg3:
        adc #6
        pha
sg4:
        ldx OPX
        lda OPY
        sec
        sbc #24
        tay
       
	jsr WAITSYNC ; Wait for vert sync
        pla
        jsr drawsprite 

        lda PF
        lsr a
        lsr a
        and #3
        tax
        lda frames,x
        clc        
        ldx PLAD
        bne sg5
                
        adc PDIR
        pha
        jmp sg6
sg5:
        adc #6
        pha
sg6:
        ldx PX
        lda PY
        sec
        sbc #24
        tay
        lda PF
        
        pla
 	jsr drawsprite 
        jsr runsprites
        jsr eventr
sg7:
        lda PX   
        sta OPX
        lda PY
        sta OPY
        lda PF
        sta OPF
        lda PDIR
        sta OPDIR
        lda PLAD
        sta OPLAD
        
        lda PY
        clc
        adc PDY
        sta PY
        lda PJ
        beq sg8
        
        lda PX
        sta TX
        inc TX
        inc TX        
        lda PY
        sec
        sbc #16
        sta TY
        jsr testmap
        beq sg8
        
        lda PY
        sec
        sbc PDY
        sta PY
        lda #0
        sta PJ
        sec
        sbc PDY
        sta PDY
sg8:
        lda PLAD
        bne nofall

        lda PDY
        bmi nofall

        lda PY
        sta TY

        ldx PX
        inx
        stx TX
        
        inc LADSOLID        
dotest:
	jsr testmap               
        bne sg9

        lda PY
        sta TY

        ldx PX
        inx
        inx
        stx TX
        
        jsr testmap
        bne sg9        

        lda PJ
        bne nofall
        
        lda PDY
        bne fallon
        sta PDX
fallon:
	lda #8
        sta PDY
        sta PFALL
        bne nofall
sg9:
        lda PY
        and #$F0
        sta TMP+7
        lda OPY
        and #$F0
        cmp TMP+7
        bcs nofall

        lda #0
        sta PDY
        sta PJ
        sta PFALL
        
        lda PY
        and #$F0
        sta PY
nofall:
	lda #0
        sta LADSOLID

        lda PX
        clc
        adc PDX
        sta PX
        
        cmp #62    ; Check border right  !!!!!!!!!!!!!!!!!!!
        bcs revx

        lda PDX
        bpl chkrht
        lda #$FF
        bne dochk
chkrht:
	lda #2
dochk:
	clc
        adc PX
        sta TMP+5
        
        lda TMP+5
        sta TX
        ldx PY
        dex
        stx TY
        jsr testmap
        bne revx

        lda TMP+5
        sta TX
        lda PY
        sec
        sbc #15
        sta TY
        jsr testmap
        bne revx
                
        jmp sg10
revx:
	jsr reversex
sg10:
        lda PDX
        beq sg11
        
        inc PF
        inc PF
sg11:
        lda PJ
        beq nojump
        
        inc PDY    ;Update jump position
        bne sg12
        inc PDY
sg12:
        lda PDY
        cmp #8
        bne nojump
        lda #0
        sta PJ
        lda #1
        sta PFALL
nojump:
	lda PFALL
        ora PJ
        bne sg13
        
        lda #0
        sta PDY

        lda $b001  ;Wait for 'SHIFT'
        and #$80
	bne sg13
	
	jsr getx
	
	lda #1
	sta PJ
	lda #250
	sta PDY
	lda #0
	sta PLAD

	ldx #1
	jsr sound
	
        jmp doneflr	
sg13:
        ldx #2     ; Up-key
        jsr getkey
	bne sg14
	jsr onladder
	beq sg13a
	jmp noladd
sg13a:
        inc PX
        lda PX
        and #$FC
        sta PX
        	
	lda #1
	sta PLAD
		
	lda #$FE
	sta PDY
	lda #0
	sta PDX
	sta PJ
	sta PFALL
	inc PF
	jmp nomovex
sg14:
        ldx #3      ;Down-key
        jsr getkey
	bne sg16

        inc PY
        inc PY
	jsr onladder
	php
	dec PY	
	dec PY	
	plp
	bne noladd
	
        lda PX
        clc
        adc #1
        and #$FC
        sta PX

	lda #1
	sta PLAD
	
	lda #2
	sta PDY
	lda #0
	sta PDX
	sta PJ
	sta PFALL
	dec PF

	jsr checkfloor
	bne sg15
        jmp nomovex
sg15:
	lda #0
	sta PDY
	sta PLAD
	jmp nomovex
sg16:
        lda PLAD
        ora PJ
        ora PFALL
        bne keydone
noladd:
	lda #0
        sta PLAD
        
        lda PDY        ;Don't test Z/X if moving vertically
        bne nomovex

        ldx #0         ; Left-key
        jsr getkey
	bne sg17

        lda PX
        cmp #0         ; Left border 0/4=0
        beq sg18
        
        lda #255
        sta PDX
        lda #3
        sta PDIR        
        
        jmp keydone
sg17:
        ldx #1         ; Right-key
        jsr getkey
	bne sg18
	
	lda PX
	cmp #60        ; Right border 60/4=15
	beq sg18
	
	ldx #1
	stx PDX
	dex
	stx PDIR
	
	jmp keydone
sg18:
        lda #0
        sta PF
        sta PDX
keydone:
        
nomovex:
        lda PJ      ;Skip conveyor if jumping or falling
        ora PFALL
        bne doneflr
        
        jsr getfloor
        
        cmp #4      ; Conveyor right
        beq sg20
        cmp #5      ; Conveyor right
        beq sg20
        cmp #6      ; Conveyor right
        bne sg21
sg20:
        lda CONVMV
        cmp #2
        bcc doneflr
        inc PDX
        jmp doneflr
sg21:
        cmp #7      ; Conveyor left
        beq sg22
        cmp #8      ; Conveyor left
        beq sg22
        cmp #9      ; Conveyor left
        bne sg23
sg22:
        lda CONVMV
        cmp #2
        bcc doneflr
        dec PDX
        jmp doneflr                 
doneflr:        
sg23:
        sei
        lda CONVMV
        cmp #2
        bcc sg24
        dec CONVMV
        dec CONVMV        
sg24:
        cli

        lda PY
        cmp #176    ; Check lower then bottomline !!!!!!!!!!!!!!!
        bcc sg25
        
        inc DEAD

        ldx #4
        jsr sound
sg25:
        dec TIME    ; Decrement timer
        bne sg26
        lda #3
        sta TIME        
        lda timenum+4
        sec
        sbc #1
        sta timenum+4
        cmp #47
        bne sg26
        lda #'9'
        sta timenum+4
        lda timenum+3
        sec
        sbc #1
        sta timenum+3
        cmp #47
        bne sg26
        lda #'9'
        sta timenum+3
        lda timenum+2
        sec
        sbc #1
        sta timenum+2
        cmp #47
        beq die
sg26:
        ldx #<timestr
        ldy #>timestr
        jsr PUTSTRING

        jsr printscr ; Print score

        lda LUNCH    ; Check if got all lunchboxes
        beq finlev
               
        ldx PY
        dex
        stx TY
        
        ldx PX
        inx
        stx TX
        
        jsr testmap

        lda DEAD     ; Last live spent
        bne die
sg27:
        jmp mainl
die:
        dec LIVES      ; Decrement lives
        beq sg28       ; If last, end

        jmp gamelp
sg28:
        lda #SCRTEXT   ; Init screenmode
        jsr setmode

        ldx #<gameovr  ; Gameover, restart
        ldy #>gameovr
        jsr PUTSTRING
        jsr delay
        
        rts

;--------------------------------
;Level finished, go to next level
;--------------------------------

finlev:
	ldx #40
        jsr delay2

finlevl:
	jsr WAITSYNC

        ldx #6
        jsr sound
finlp:
	lda #8
        sta TMP+1
finlp2:
	lda timenum+2
        sec
        sbc #48
        sta TMP
        lda timenum+3
        sec
        sbc #48
        ora TMP
        sta TMP
        lda timenum+4
        sec
        sbc #48
        ora TMP
        beq timeovr
        
        lda timenum+4
        sec
        sbc #1
        sta timenum+4
        cmp #47
        bne fl1
        lda #'9'
        sta timenum+4
        lda timenum+3
        sec
        sbc #1
        sta timenum+3
        cmp #47
        bne fl1
        lda #'9'
        sta timenum+3
        lda timenum+2
        sec
        sbc #1
        sta timenum+2
fl1:
        sed
        lda SCORE
        clc
        adc #$10
        sta SCORE
        lda SCORE+1
        adc #0
        sta SCORE+1
        lda SCORE+2
        adc #0
        sta SCORE+2
        lda SCORE+3
        adc #0
        sta SCORE+3
        cld

        dec TMP+1
        bne finlp2       
        
        ldx #<timestr
        ldy #>timestr
        jsr PUTSTRING        
        
        jsr printscr

        jmp finlevl         

timeovr:
	ldx #<timestr
        ldy #>timestr
        jsr PUTSTRING        

        jsr printscr        

	jsr delay
        
        inc LEVN
        
        jmp gamelp

;--------------------------------
;Get tile at player position
;--------------------------------

getpy:
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	lsr a       ; PY/16 is tile pos
        lsr a
        lsr a
        lsr a

        asl a       ; leveldatarow = PY*16 + PY*4 = PY*20
        asl a
        sta TMP
        
        asl a
        asl a
        rol ADDR+1
        clc
        adc TMP
        bcc gp1
        inc ADDR+1
gp1:
        clc
        adc #<level        
        sta ADDR
        lda ADDR+1
        adc #>level
        sta ADDR+1
       
        lda PX      ; leveldatacol = PX/4
        clc
        adc #1
        lsr a
        lsr a
        tay
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        
        lda (ADDR),y
        rts

;--------------------------------
;If Harry is on ladder
;--------------------------------

onladder:
	lda #0
        sta ADDR+1
        lda PY
        sec
        sbc #1
        jsr getpy
        cmp #3
        rts
        
;--------------------------------
;Floor routines
;--------------------------------

getfloor:
	lda #0
        sta ADDR+1
        lda PY
        jsr getpy        
        rts

checkfloor:
	jsr getfloor
        cmp #3
        rts

;--------------------------------
;Test floor under Harry
;--------------------------------

testmap:
        lda #0
        sta ADDR
        sta ADDR+1
        
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        lda TY         ; TY/16 is tilepos
        lsr a
        lsr a
        lsr a
        lsr a
        sta TY         ; leveldatarow = TY*16 + TY*4 = TY*20
        asl a
        asl a
        sta TMP
        
        asl a
        asl a
        rol ADDR+1
        clc
        adc TMP
        bcc tm1
        inc ADDR+1
tm1:
        clc
        adc #<level        
        sta ADDR
        lda ADDR+1
        adc #>level
        sta ADDR+1      

        lda TX         ; levelcolumn = TX/4
        lsr a
        lsr a
        sta TX
        tay        
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

testsolid:
	lda (ADDR),y
        cmp #3        
        beq tm2
        cmp #10
        beq getlunch                            
        tay
        lda solid,y

        rts
tm2:
        lda LADSOLID
        beq itszero
        lda ADDR
        sec
        sbc #SCR_W
        sta ADDR
        bcs tm3
        dec ADDR+1
tm3:
        lda (ADDR),y
        beq tm4
itszero:
	lda #0
        rts
tm4:
        lda #1 
        rts

;--------------------------------
;Remove lunchbox and increment score
;--------------------------------

getlunch:
	lda #0
        sta (ADDR),y

	lda TX
	asl a
	sta ADDR
	lda TY
	asl a
	clc
	adc #>TOPSCR
	sta ADDR+1

        lda #10
        jsr drawtile
        
        sed
        lda SCORE+1
        clc
        adc #2
        sta SCORE+1
        lda SCORE+2
        adc #0
        sta SCORE+2
        lda SCORE+3
        adc #0
        sta SCORE+3
        cld        

        dec LUNCH
        
        ldx #5
        jsr sound

        lda #0
                        
        rts

;--------------------------------
;Print highscore list
;--------------------------------

hiscores:
        lda #0
        sta TMP
scrlp:
        lda TMP
        clc
        adc #6
	sta hiline+1
        
        cmp #15
	bne not10
        
        lda #49
        sta hiline+2
        lda #48
        sta hiline+3
        jmp scr1
        
not10:  lda #32
        sta hiline+2
        lda TMP
        clc
        adc #49
        sta hiline+3
scr1:
        ldx TMP
        lda hiname1,x
        sta hiline+7
        lda hiname2,x
        sta hiline+8
        lda hiname3,x
        sta hiline+9

        lda hibyte1,x
        sta NUM
        lda hibyte2,x        
        sta NUM+1
        lda hibyte3,x       
        sta NUM+2
        lda hibyte4,x       
        sta NUM+3

        jsr printnum       

	ldx #<hiline
	ldy #>hiline
	jsr PUTSTRING

        inc TMP
        lda TMP
        cmp #10
        bne scrlp
        rts

printnum:
        ldy #0
	ldx #3
prn1:
        lda NUM,x
	jsr getnum1
	sta hiline+12,y
	iny	
        lda NUM,x
        and #15
        clc
        adc #48
	sta hiline+12,y
	iny
	dex
	bpl prn1
	rts

getnum1:         
        lsr a
        lsr a
        lsr a
        lsr a
        clc
        adc #48
        rts

;--------------------------------
;Set mode and clear screen
;--------------------------------

setmode:
        sta $b000
	lda #<$8000
	sta TMP
	lda #>$8000
	sta TMP+1
	jmp sm1
clrscr:
	lda #<TOPSCR
	sta TMP
	lda #>TOPSCR
	sta TMP+1
sm1:
	lda #0
	tay
sm2:
	sta (TMP),y
	iny
	bne sm2
	inc TMP+1
	lda TMP+1
	cmp #$98
	bne sm1
	rts

;--------------------------------
;Reverse X-dir
;--------------------------------

reversex:
	lda PX 
        sec
        sbc PDX
        sta PX
        
        lda #0
        sec
        sbc PDX
        sta PDX
        rts         

;--------------------------------
;Get Z- or X-key
;--------------------------------

getx:
	ldx #0 ;'Z'
        jsr getkey
	bne gkx1
	
	lda #255
	sta PDX
	lda #3
	sta PDIR
	rts
gkx1:
        ldx #1 ;'X'
        jsr getkey
	bne gkx2
        
        lda #1
        sta PDX
        lda #0
        sta PDIR
        rts
gkx2:
        lda #0
        sta PDX
        rts         	

;--------------------------------
;Getkey and store result in KEY
;--------------------------------

getkey:
        stx TMP
	jsr OSRDKEY
	sty KEY
	tya
	ldx TMP
	cmp keys,x
	rts

;            Z   X   ;;  >. RET SPC
keys: .BYTE $3a,$38,$1b,$1e,$0d,$00

;--------------------------------
;Find highscore
;--------------------------------

findhi: ldx #9
        sed
fh1:
        lda hibyte1,x
        sec
        sbc SCORE
        lda hibyte2,x
        sbc SCORE+1
        lda hibyte3,x
        sbc SCORE+2
        lda hibyte4,x
        sbc SCORE+3
        bpl fh2
        
        dex
        bpl fh1
        
        ldx #$FF
fh2:
        inx
        cld
        rts

;--------------------------------
;Print levelnumber
;--------------------------------

levnum:
	lda LEVN
        clc
        adc #1
        ldx #0
ln1:
        cmp #10
        bcc ln2
        inx
        sec
        sbc #10
        jmp ln1

ln2:
        pha
        txa
        beq ln3
        clc
        adc #48
	ldx XPOS
	ldy YPOS
        jsr PUTCHAR
        
ln3:
        pla
        clc
        adc #48
	ldx XPOS
	inx
	ldy YPOS
        jsr PUTCHAR
        
        rts

;--------------------------------
;Move score from X down        
;--------------------------------

movehi:
	cpx #9
        bne mh1
        rts
mh1:
	stx TMP
        ldy #8
mh2:
        lda hiname1,y
        sta hiname1+1,y
        lda hiname2,y
        sta hiname2+1,y
        lda hiname3,y
        sta hiname3+1,y
        lda hibyte1,y
        sta hibyte1+1,y
        lda hibyte2,y
        sta hibyte2+1,y
        lda hibyte3,y
        sta hibyte3+1,y
        lda hibyte4,y
        sta hibyte4+1,y
        
        dey
        bmi mh3
        
        cpy TMP
        bcs mh2
mh3:
        rts

;--------------------------------
;Get character
;--------------------------------

getchr:
	jsr OSRDCH
        cmp #32    ; SPACE
        bcc getchr
	cmp #$7f   ; DELETE
        rts

;--------------------------------
;Delay 2 sec
;--------------------------------
delay:
	ldx #120
delay2:
	jsr WAITSYNC
	dex
	bne delay2
        rts

;--------------------------------
;Init sprites
;--------------------------------


;--------------------------------
;Draw tile 2 x 16 bytes
;--------------------------------
;	Address in ADDR
;	Tile number in A
;--------------------------------

drawtile:
	stx XPOS

        ldy #0
        sty TILE
        lsr a
        ror TILE
        lsr a
        ror TILE
        lsr a
        ror TILE
        sta TILE+1
        
        lda TILE
        clc
        adc #<tiles
        sta TILE
        lda TILE+1
        adc #>tiles
        sta TILE+1

	lda ADDR
	pha
	lda ADDR+1
	pha
        
	ldx #15
dt1:
        ldy #0
        lda (TILE),y
        eor (ADDR),y
        sta (ADDR),y
        ldy #8
        lda (TILE),y
	ldy #1
        eor (ADDR),y
        sta (ADDR),y
	inc TILE
	bne dt4
	inc TILE+1
dt4:
	clc
	lda ADDR
	adc #$20
	sta ADDR
	lda ADDR+1
	adc #0
	sta ADDR+1

        cpx #8
	bne dt5
	lda TILE
	clc
	adc #8
	sta TILE
	lda TILE+1
	adc #0
	sta TILE+1
dt5:
        dex
        bpl dt1
        
	ldx XPOS
	pla
	sta ADDR+1
	pla
	sta ADDR
        rts

;--------------------------------
;Run sprites
;--------------------------------

runsprites:
	lda #15
        sta TMP
sprloop:
	ldx TMP
        lda SPRX,x
        bmi nospr
        
        lda SPRX,x
        sta SPRTMP+1
        lda SPRY,x
        sta SPRTMP+2
        
        ldy SPRT,x
        lda sprrouts,y
        sta jsrmod+1
        lda sprrouts+1,y
        sta jsrmod+2
jsrmod:
	jsr $ffff

        ldx TMP
        lda SPRT,x
        pha
        ldy SPRY,x
        lda SPRX,x
        tax
        pla
        jsr coldetect        
nospr:
	dec TMP
        bpl sprloop
        
        rts

;--------------------------------
;Detect collision
;--------------------------------

coldetect:
	stx TMP+1
        pha
        ldx TMP
        lda SPRT,x
        tax
        lda colsizes,x
        lsr a
        adc TMP+1
        sec
        sbc PX
        sec
        sbc #1
        bpl cd1
        eor #$FF
        clc
        adc #1
cd1:
        sta TMP+1

        sty TMP+2
        lda colsizes+1,x
        lsr a
        adc TMP+2

        sec
        sbc PY
        clc
        adc #12
        bpl cd2
        eor #$FF
        clc
        adc #1
cd2:
        sta TMP+2

        pla
        tax
        lda coldiffs,x
        cmp TMP+1
        bcc cd3
        lda coldiffs+1,x
        asl a
        cmp TMP+2
        bcc cd3
        
        lda #1
        sta DEAD

        ldx #3
        jsr sound
	ldx #40
	jsr delay2
cd3:
        rts

colsizes: .BYTE 6,16,6,8,2,16,6,16
coldiffs: .BYTE 6,16,2,8,2,8, 4,16

;--------------------------------
;Drawsprite24 - A=sprite num, X=xpos, Y=ypos
;	Sprite 3 x 32
;--------------------------------

drawsprite24:
        jsr spraddr 
        ldx #32
sprlp24:
        ldy #0
        lda (SPR),y
        eor (ADDR),y
        sta (ADDR),y
        
        ldy #32        
        lda (SPR),y
        ldy #1
        eor (ADDR),y
        sta (ADDR),y
        
        ldy #64        
        lda (SPR),y
        ldy #2
        eor (ADDR),y
        sta (ADDR),y

        jsr sprcmn
        bne sprlp24
        
        rts

;--------------------------------
;Drawsprite24 - A=sprite num, X=xpos, Y=ypos
;	Sprite 3 x 32
;--------------------------------

drawsprite24_16:
	jsr spraddr
        ldx #16
        
sprlp24_16:ldy #0
        lda (SPR),y
        eor (ADDR),y
        sta (ADDR),y
        
        ldy #16        
        lda (SPR),y
        ldy #1
        eor (ADDR),y
        sta (ADDR),y
        
        ldy #32        
        lda (SPR),y
        ldy #2
        eor (ADDR),y
        sta (ADDR),y

        jsr sprcmn
        bne sprlp24_16
        
        rts

;--------------------------------
;Drawsprite24 - A=sprite num, X=xpos, Y=ypos
;	Sprite 3 x 12 bytes
;--------------------------------

drawsprite24_12:
	jsr spraddr         

        ldx #12
sprlp24_12:
	jsr spr12in
        jsr sprcmn
        bne sprlp24_12

        rts
spr12in:
	ldy #0
        lda (SPR),y
        eor (ADDR),y
        sta (ADDR),y
        
        ldy #12        
        lda (SPR),y
        ldy #1
        eor (ADDR),y
        sta (ADDR),y
        
        ldy #24        
        lda (SPR),y
        ldy #2
        eor (ADDR),y
        sta (ADDR),y
                
        rts
        
;--------------------------------
;Drawsprite32 - A=sprite num, X=xpos, Y=ypos
;	Sprite 3 x 12
;--------------------------------

drawsprite32:
	jsr spraddr        
        ldx #12
sprlp32:
	jsr spr12in
        ldy #36        
        lda (SPR),y
        ldy #3
        eor (ADDR),y
        sta (ADDR),y

        ldy #48        
        lda (SPR),y
        ldy #4
        eor (ADDR),y
        sta (ADDR),y

        ldy #60        
        lda (SPR),y
        ldy #5
        eor (ADDR),y
        sta (ADDR),y

        jsr sprcmn
                
        bne sprlp32
        
       rts

;--------------------------------
;Drawsprite16 - A=sprite num, X=xpos, Y=ypos
;	Sprite 2 x 16 bytes
;--------------------------------

drawsprite16:
	jsr spraddr        
        ldx #16
sprlp16:
	ldy #0
        lda (SPR),y
        eor (ADDR),y
        sta (ADDR),y
        
        ldy #16        
        lda (SPR),y
        ldy #1
        eor (ADDR),y
        sta (ADDR),y
        
        jsr sprcmn
               
        bne sprlp16
        
        rts

;--------------------------------
;Drawsprite- A=sprite num, X=xpos, Y=ypos
;	Sprite 3 x 24 bytes
;--------------------------------

drawsprite:
	pha
        lda #0
        sta TMP+6
        sta TMP+7
        sta SPR+1
        txa
        and #1
        sta TMP+5 ; Spriteframe 0 (x=even) or 1 (x=odd)
        
        pla
        asl a
        ora TMP+5
        pha
        ;Now need *72 - *8 + *64
        lsr a
        ror TMP+6
        lsr a
        ror TMP+6
        sta TMP+7 ;*64
        
        pla
        asl a
        rol SPR+1
        asl a
        rol SPR+1
        asl a
        rol SPR+1 ;*8
        clc
        adc TMP+6
        sta SPR
        lda SPR+1
        adc TMP+7
        sta SPR+1
        
        jsr spraddr2

        lda #<sprites
        clc
        adc SPR
        sta SPR
        lda #>sprites
        adc SPR+1
        sta SPR+1
        
        ldx #24
sprlp:
	ldy #0
        lda (SPR),y
        eor (ADDR),y
        sta (ADDR),y
        
        ldy #24        
        lda (SPR),y
        ldy #1
        eor (ADDR),y
        sta (ADDR),y
        
        ldy #48        
        lda (SPR),y
        ldy #2
        eor (ADDR),y
        sta (ADDR),y
        
        jsr sprcmn

        bne sprlp
        
        rts

;--------------------------------
;Calculate Sprite-/screenaddresses
;X is a multiple of 2 in bytes
;Y is line from #8100
;--------------------------------

spraddr:
        stx TMP+6

        asl a
        tax
        lda spr24,x
        sta SPR
        lda spr24+1,x
        sta SPR+1
        
        ldx TMP+6

spraddr2:
        lda #0
        sta ADDR
        sta ADDR+1
        sta TMP+7
        
        sty ADDR
	asl ADDR
	rol ADDR+1
	asl ADDR
	rol ADDR+1
	asl ADDR
	rol ADDR+1
	asl ADDR
	rol ADDR+1
	asl ADDR
	rol ADDR+1
	lda ADDR+1
	clc
	adc #>(TOPSCR)
	sta ADDR+1

        lda #0
        sta TMP+7        
        txa
        lsr a
        clc
        adc ADDR
        sta ADDR
        lda ADDR+1
        adc #0
        sta ADDR+1

        rts

;--------------------------------
;Increment SPR and ADDR pointers
;	SPR+1
;	ADDR+$20
;--------------------------------

sprcmn:
        inc SPR        
        bne sprc1
        inc SPR+1
sprc1:
        lda ADDR
        clc
        adc #$20
        sta ADDR
	lda #0
        adc ADDR+1
	sta ADDR+1
        dex
        rts

;--------------------------------
;Draw level
;--------------------------------

drawlevel:
	lda #SCR_H-4     ;Screen height
        sta TMP+3
        
        ldx #0
        stx LUNCH
        
        lda #<TOPSCR
        sta ADDR
        pha
        lda #>TOPSCR
        sta ADDR+1
        pha
        lda #<level
        sta LEV
        lda #>level
        sta LEV+1
drawlp:
	txa
        tay
        lda (LEV),y
        cmp #10
        bne dl1
        inc LUNCH        
dl1:
        jsr drawtile

        lda ADDR
        clc
        adc #2      ;Next tile
        sta ADDR
        bcc dl2
        inc ADDR+1
dl2:
        inx
        cpx #SCR_W-4     ;Screen width
        bne drawlp

        lda LEV
        clc
        adc #SCR_W
        sta LEV
        bcc dl3
        inc LEV+1
dl3: 
        pla
	clc
        adc #2
        sta ADDR+1
        pla
        sta ADDR
        
        lda ADDR
        pha
        lda ADDR+1
        pha        

        ldx #0
        dec TMP+3
        bne drawlp
        
        pla
        pla
        
        rts
 
;--------------------------------
;load level into buffer
;--------------------------------

loadlevel:
	lda LEVN
        and #15
        
        asl a
        tax
        lda levs,x
        sta loadmod+1
        lda levs+1,x
        sta loadmod+2
        
        lda sprs,x
        sta SPRADR
        lda sprs+1,x
        sta SPRADR+1
        
        lda #<level
        sta TMP
        lda #>level
        sta TMP+1
        
        ldx #0
        ldy #0
loadmod:
	lda $ffff,x
        beq ldend
        pha
        lsr a
        lsr a
        lsr a
        lsr a
        sta TMP+2
        pla
        and #$F
ll1:
        sta (TMP),y
        inc TMP
        bne ll2
        inc TMP+1
ll2:
        dec TMP+2
        bne ll1

        inx
        
        bne loadmod
                
ldend:  lda LEVN
        and #15
        cmp #9
        bne ll3
        
        lda #0
        sta level+$1a
        sta level+$1b
        sta level+$1c
        sta level+$1d
        
        lda #1
        sta level+$2e
        sta level+$31
ll3:
        rts

;--------------------------------
;Place initial sprites at startup
;--------------------------------

initsprites:
	ldx #15
        lda #255
is1:
        sta SPRX,x       ; Sprites not visible
        sta SPR1,x
        dex
        bpl is1

        ldy #0
        ldx #0
insprlp:
	lda (SPRADR),y   ; Read X-pos and D1
	sta SPRTMP
	iny
	lda (SPRADR),y   ; Read Y-pos + spritenr
	sta SPRTMP+1
	iny
	lda (SPRADR),y   ; Read direction and D2
	sta SPRTMP+2
	iny
                    
	lda SPRTMP
	bpl sprc
	cmp #$ff         ; $ff is end of list
	beq sprfin
	lda LEVN         ; Check if level>15
	and #$F0
	beq sprskp
	lda SPRTMP
	and #$7f
sprc:
	sta SPRX,x
	sta SPRD1,x
	lda SPRTMP+1     ; Filter Y-pos
	and #$F8
	sta SPRY,x
	lda SPRTMP+1     ; Filter Type
	and #7
	asl a
	sta SPRT,x       ; Type x 2
	lda #1           ; Set Dir = 1
	sta SPRD,x
          
	lda SPRTMP+2
	bpl is2          ; If Direction bit set Dir = 255
	dec SPRD,x
	dec SPRD,x
is2:
	and #$7F
	sta SPRD2,x
sprskp:
	inx
	jmp insprlp
sprfin:
	rts

;--------------------------------
;Panda routine
;--------------------------------

pandarout:
	lda SPR1,x
        beq pr1
        
        lda #0
        sta SPR1,x
        
        lda #16
        ldx SPRTMP+1
        ldy SPRTMP+2
        jsr drawsprite24
pr1:
        rts        

;--------------------------------
;Big Kong routines
;--------------------------------

bkrout:
	lda SPR1,x
        beq bk1

        lda SPRTMP+1
        clc
	adc #8
	tax
        lda SPRTMP+2
        clc
	adc #8
	tay
        lda #9          ; Barrel
        jsr drawsprite
bk1:
        ldx TMP
        lda SPR1,x
        bne bkfirst
        
        dec BKTIME
        bne bk2

        lda #16
        sta BKTIME
                
        lda BKFRAME
        and #1
        ldx SPRTMP+1
        ldy SPRTMP+2
        jsr drawsprite24

        inc BKFRAME
bkfirst:
	ldx TMP
        lda #0
        sta SPR1,x
        
        lda BKFRAME
        and #1
        ldx SPRTMP+1
        ldy SPRTMP+2
        jsr drawsprite24
bk2:       
        lda BARRELX
        bmi nobarrel

        lda #3
        ldx BARRELX
        ldy BARRELY
        jsr drawsprite24_16

        jsr barrel_move
                
        lda BARRELY
        cmp #16
        bcc killbarrel
        cmp #168          ;!!!!!!!!!!!!!!!! was 232 !!!!!!!!!!!!!!
        bcs killbarrel
        
        lda BARRELX
        cmp #68
        bcs killbarrel
        
        lda BARRELX
        clc
        adc #3
        sec
        sbc PX
        sec
        sbc #1
        bpl bk3
        eor #$FF
        clc
        adc #1
bk3:
        sta TMP+1

        lda BARRELY
        clc
        adc #2
        sec
        sbc PY
        clc
        adc #12
        bpl bk4
        eor #$FF
        clc
        adc #1
bk4:
        sta TMP+2

        lda #6
        cmp TMP+1
        bcc bk5
        lda #6
        cmp TMP+2
        bcc bk5
        
        lda #1
        sta DEAD
bk5:
        lda #3
        ldx BARRELX
        ldy BARRELY
        jsr drawsprite24_16
        jmp bk6

killbarrel:
	lda #255
        sta BARRELX        
bk6:
nobarrel:        
        dec BARRELT
        bne bk8        

        lda #160
        sta BARRELT
        lda LEVN
        cmp #32
        bcc bk7
        lda #96
        sta BARRELT
bk7:
        ldx SPRX
        stx BARRELX
        ldy #32
        sty BARRELY
        lda #3
        jsr drawsprite24_16
        jsr barrel_dir

	ldx #3
	jsr sound
	ldx #40
	jsr delay2
bk8:
        dec HAYLPTIME
        bne bk9
        
        lda #50
        sta HAYLPTIME

        lda SPRX
        clc
        adc #12
        tax        
        lda #2
        ldy #0
        jsr drawsprite32
bk9:
        rts

barrel_move:
	lda BARRELD
        beq barrel_xl
        cmp #1
        beq barrel_yl
        cmp #2
        beq barrel_yr

        inc BARRELX
        inc BARRELX
        bne bm1
barrel_xl:
	dec BARRELX
        dec BARRELX
bm1:
        lda BARRELE
        sec
        sbc BARRELDY
        sta BARRELE
        bcs bm2        
        
        lda BARRELE
        adc BARRELDX
        sta BARRELE
                
        lda BARRELY
	clc
	adc #8
	sta BARRELY
bm2:
        rts
        
barrel_yl:
	jsr barrelc1

        bcs byl1
        
        lda BARRELE
        adc BARRELDY
        sta BARRELE        
        
        dec BARRELX
        dec BARRELX
byl1:
        rts
        
barrel_yr:
	jsr barrelc1

        bcs byr1
        
        lda BARRELE
        adc BARRELDY
        sta BARRELE        

        inc BARRELX
        inc BARRELX        
byr1:
        rts

barrelc1:
	lda BARRELY
        clc
        adc #8
        sta BARRELY
        
        lda BARRELE
        sec
        sbc BARRELDX
        sta BARRELE

        rts
        
barrel_dir:
	lda #0
        sta BARRELE
        
        lda BARRELX
        sec
        sbc PX
        bcs bd1
        eor #$ff
        clc
        adc #1
bd1:
        sta BARRELDX

        lda BARRELY
        sec
        sbc PY
        bcs bd2
        eor #$ff
        clc
        adc #1
bd2:
        lsr a
        lsr a
        sta BARRELDY

        lda BARRELDX
        cmp BARRELDY
        bcs barrel_x
        
        ldx #1
        lda BARRELX
        cmp PX
        bcs bd3
        inx
bd3:
        stx BARRELD
        rts                
        
barrel_x:
	ldx #0
        lda BARRELX
        cmp PX
        bcs bx1
        ldx #3
bx1:
        stx BARRELD
        rts

;--------------------------------
;Bird routine
;--------------------------------

birdrout:
	lda SPR1,x
        bne bro1

        jsr sprgetfr
        adc #4

        jsr getdirfr
        ldx SPRTMP+1
        ldy SPRTMP+2
        jsr drawsprite24_12
bro1:
        ldx TMP
        lda #0
        sta SPR1,x
        
        inc SPRF,x
        
        lda SPRX,x
        clc
        adc SPRD,x
        sta SPRX,x
        cmp SPRD1,x
        beq birdrev
        cmp SPRD2,x
        bne birdcnt

birdrev:lda #0
        sec
        sbc SPRD,x
        sta SPRD,x

birdcnt:jsr sprgetfr
        adc #4

        jsr getdirfr
        ldy TMP
        ldx SPRX,y
        
        ldy SPRTMP+2        
        jsr drawsprite24_12

        rts

sprgetfr:
	ldx TMP
        lda SPRF,x
        lsr a
        and #3
        tay
        lda frlook,y
        sta SPRTMP+3
        clc

        rts

getdirfr:
	ldy SPRD,x
        bmi gdf1
        clc
        adc #3
gdf1:
        rts

frlook:
	.db 0,1,0,2

;--------------------------------
;Monkey routine
;--------------------------------

monkeyrout:
	jsr monkeygetfr 
        
        lda SPR1,x
        beq mr2
        
        lda #0            ;If first run then draw monkey
        sta SPR1,x        ;Monkey doesn't move or animate
        jsr rand
        and #63
        adc #64
        sta SPRD3,x
        lda LEVN
        cmp #16
        bcc mr1
       
        lda SPRD3,x
        sec
        sbc #24       
        sta SPRD3,x
mr1:
	lda #255
        sta SPRD1,x
        
        lda SPRD,x
        and #2
        lsr a
        ora #10
        ldx SPRTMP+1
        ldy SPRTMP+2
        jsr drawsprite16
        jmp nospan
mr2:
        dec SPRD3,x       ;Decrement spanner counter
        bne mr3
        
        jsr rand          ;Reset counter and generate spanner
        and #63
        adc #64        

        sta SPRD3,x
        
        lda SPRY,x
        sta SPRD2,x
        tay
        lda SPRD,x
        asl a
        clc
        adc SPRX,x
        sta SPRD1,x
        tax
        lda TMP+1
        jsr drawsprite16
        
        ldx #2
        jsr sound

        jmp nospan
mr3:
        lda SPRD1,x      ;Is spanner on screen?
        bmi nospan

        ldy SPRD2,x
        lda SPRD1,x
        tax
        lda TMP+1
        jsr drawsprite16
        ldx TMP
        inc SPRF,x
        jsr monkeygetfr
        ldy SPRD2,x        
        lda SPRD,x
        asl a
        clc
        adc SPRD1,x
        sta SPRD1,x
        bmi spanover
        cmp  #60     ; Check border right !!!!!!!!!!!!!
        bcs spanover        
        
        tax
        lda TMP+1
        jsr drawsprite16

        ldx TMP
        lda SPRD1,x
        clc
        adc #2
        sec
        sbc PX
        sec
        sbc #1
        bpl mr4
        eor #$FF
        clc
        adc #1
mr4:
        sta TMP+1

        lda SPRD2,x
        clc
        adc #8
        sec
        sbc PY
        clc
        adc #12
        bpl mr5
        eor #$FF
        clc
        adc #1
mr5:
        sta TMP+2

        lda #4
        cmp TMP+1
        bcc nospan
        lda #16
        cmp TMP+2
        bcc nospan
        
        lda #1
        sta DEAD

        ldx #3
        jsr sound
	ldx #40
	jsr delay2

nospan:        
        rts

spanover:
	lda #255
        sta SPRD1,x
        rts

monkeygetfr:
	lda SPRF,x
        lsr a
        and #3
        ora #12
        sta TMP+1
        rts
                
;--------------------------------
;Random???
;--------------------------------

rand:
	lda SEED
        and #$48
        adc #$38
        asl a
        asl a
        rol SEED+2
        rol SEED+1
        rol SEED
        lda SEED
        rts
        
SEED: .BYTE $A6,$73,$9F

;--------------------------------
;Routine is executed every frame
;--------------------------------

eventr:
        dec COLTIME
        bne ev1
        
        tya
        pha
        
        lda #2
        sta COLTIME
        
        inc CONVMV
        
        inc COLCOUNT
        lda COLCOUNT
        and #3
        sta COLCOUNT
        tay
        
        pla
        tay
ev1:
        rts

;--------------------------------
;Level 18      
;--------------------------------

level1: .BYTE $90,$13,$60,$40
        .BYTE $90,$13,$60,$40
        .BYTE $20,$1A,$20,$1A,$30,$41,$30,$40
        .BYTE $10,$13,$41,$13,$90,$40
        .BYTE $10,$13,$40,$13,$1A,$70,$1A,$40
        .BYTE $10,$13,$40,$21,$13,$20,$13,$51,$30
        .BYTE $10,$13,$60,$13,$20,$13,$40,$40
        .BYTE $10,$13,$10,$1a,$10,$1a,$20,$13,$20,$13,$40,$40
        .BYTE $21,$10,$11,$13,$11,$10,$13,$51,$13,$20,$40
        .BYTE $40,$13,$20,$13,$40,$1A,$13,$10,$1A,$40
        .BYTE $20,$61,$40,$41,$40
        .BYTE $B0,$61,$30
        .BYTE $F0,$50
        .BYTE $40,$13,$20,$13,$50,$13,$20,$40
        .BYTE $30,$1A,$10,$1A,$20,$13,$20,$13,$40,$40

        ;Level 1
level2: .BYTE $f0,$50
        .BYTE $30,$1A,$40,$1A,$40,$1A,$20,$40
        .BYTE $30,$41,$13,$11,$30,$31,$13,$40
        .BYTE $70,$13,$70,$13,$40
        .BYTE $60,$1A,$13,$1A,$40,$1A,$10,$13,$40
        .BYTE $50,$13,$31,$30,$21,$13,$11,$40
        .BYTE $50,$13,$80,$13,$10,$40
        .BYTE $50,$13,$80,$13,$10,$40
        .BYTE $11,$13,$30,$13,$70,$21,$13,$40
        .BYTE $1A,$13,$20,$1A,$13,$20,$1A,$60,$13,$40
        .BYTE $31,$10,$51,$20,$51,$40
        .BYTE $f0,$50
        .BYTE $f0,$50
        .BYTE $f0,$50
        .BYTE $f0,$50

level3: .BYTE $F0,$10,$40
        .BYTE $30,$1A,$10,$1A,$80,$1A,$10,$40
        .BYTE $10,$13,$51,$13,$40,$13,$31,$40
        .BYTE $10,$13,$50,$13,$40,$13,$30,$40
        .BYTE $10,$13,$40,$1A,$13,$40,$13,$1A,$20,$40
        .BYTE $10,$13,$40,$21,$30,$31,$13,$10,$40
        .BYTE $10,$13,$C0,$13,$10,$40
        .BYTE $10,$13,$30,$1A,$40,$1a,$10,$1A,$10,$13,$10,$40
        .BYTE $21,$30,$11,$13,$20,$13,$51,$10,$40
        .BYTE $20,$1a,$30,$13,$1A,$10,$13,$60,$40
        .BYTE $31,$30,$41,$60,$40
        .BYTE $F0,$50,$40
        .BYTE $F0,$50,$40
        .BYTE $F0,$50,$40
        .BYTE $F0,$50,$40
        
level4: .BYTE $F0,$50
        .BYTE $1A,$90,$1A,$40,$1A,$40
        .BYTE $11,$13,$10,$1A,$60,$11,$13,$20,$13,$11,$40
        .BYTE $10,$11,$13,$11,$10,$17,$18,$19,$30,$13,$10,$1A,$13,$10,$40
        .BYTE $20,$13,$80,$11,$13,$21,$10,$40
        .BYTE $11,$13,$11,$14,$45,$16,$30,$13,$30,$40
        .BYTE $10,$13,$a0,$13,$30,$40
        .BYTE $10,$13,$1A,$20,$1A,$60,$13,$30,$40
        .BYTE $13,$21,$10,$31,$10,$17,$18,$19,$13,$21,$20,$40
        .BYTE $13,$20,$1A,$10,$1A,$50,$13,$10,$1A,$20,$40
        .BYTE $71,$30,$41,$20,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        
level5: .BYTE $F0,$50
        .BYTE $2A,$10,$70,$30,$1A,$10,$1A,$40
        .BYTE $21,$13,$30,$31,$40,$11,$13,$11,$40
        .BYTE $20,$13,$70,$1A,$20,$1a,$13,$1A,$40
        .BYTE $20,$13,$20,$2A,$20,$13,$11,$20,$21,$13,$40
        .BYTE $20,$13,$20,$13,$11,$20,$13,$10,$1a,$30,$13,$40
        .BYTE $50,$13,$30,$31,$13,$10,$1a,$13,$40
        .BYTE $30,$13,$21,$60,$13,$10,$21,$40
        .BYTE $11,$13,$10,$13,$80,$13,$30,$40
        .BYTE $1A,$13,$10,$13,$10,$1A,$10,$1A,$10,$1a,$10,$1A,$13,$10,$1A,$10,$40
        .BYTE $41,$10,$11,$10,$31,$10,$41,$10,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        
level6: .BYTE $F0,$50
        .BYTE $70,$2A,$70,$40
        .BYTE $60,$13,$21,$13,$60,$40
        .BYTE $60,$13,$20,$13,$60,$40
        .BYTE $30,$13,$11,$1A,$41,$1A,$11,$13,$30,$40
        .BYTE $30,$13,$80,$13,$30,$40
        .BYTE $30,$13,$1A,$60,$1A,$13,$30,$40
        .BYTE $20,$13,$31,$1A,$21,$1A,$31,$13,$20,$40
        .BYTE $11,$10,$13,$A0,$13,$20,$40
        .BYTE $20,$13,$1A,$20,$1A,$20,$1A,$20,$1A,$13,$20,$40
        .BYTE $10,$31,$20,$41,$20,$31,$10,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
 
level7: .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $11,$1A,$13,$10,$14,$15,$16,$1A,$17,$19,$10,$1A,$11,$20,$13,$40
        .BYTE $20,$13,$C0,$13,$40
        .BYTE $20,$13,$C0,$13,$40
        .BYTE $1A,$31,$14,$16,$1A,$17,$18,$19,$1A,$21,$20,$13,$40
        .BYTE $F0,$13,$40
        .BYTE $F0,$13,$40
        .BYTE $11,$17,$18,$19,$1A,$11,$1A,$14,$15,$16,$10,$17,$19,$20,$13,$40
        .BYTE $F0,$13,$40
        .BYTE $14,$C5,$15,$16,$11,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50

level8: .BYTE $F0,$50
        .BYTE $1A,$80,$1A,$50,$11,$40
        .BYTE $11,$40,$11,$1A,$10,$21,$20,$1a,$30,$40
        .BYTE $20,$21,$20,$21,$40,$21,$10,$1a,$40
        .BYTE $a0,$1a,$30,$11,$13,$40
        .BYTE $11,$30,$21,$40,$21,$30,$13,$40
        .BYTE $60,$1a,$80,$13,$40
        .BYTE $60,$11,$80,$13,$40
        .BYTE $11,$C0,$1A,$10,$13,$40
        .BYTE $1A,$20,$21,$10,$1A,$10,$21,$50,$13,$40
        .BYTE $21,$30,$21,$40,$21,$10,$21,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        
level9: .BYTE $F0,$50
        .BYTE $A0,$1A,$50,$40
        .BYTE $10,$11,$13,$22,$11,$20,$11,$22,$14,$15,$16,$1A,$11,$40
        .BYTE $20,$13,$22,$20,$13,$10,$22,$50,$40
        .BYTE $20,$13,$22,$20,$13,$10,$22,$20,$1A,$20,$40
        .BYTE $30,$22,$20,$13,$1A,$22,$20,$31,$40
        .BYTE $20,$13,$22,$20,$21,$22,$50,$40
        .BYTE $20,$13,$22,$13,$30,$22,$1A,$30,$1A,$40
        .BYTE $11,$10,$13,$22,$13,$30,$22,$14,$15,$16,$10,$21,$30
        .BYTE $1A,$10,$13,$22,$13,$2A,$10,$22,$1A,$40,$40
        .BYTE $31,$22,$31,$10,$22,$51,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50

level10:.BYTE $70,$22,$70,$40
        .BYTE $70,$22,$30,$1A,$30,$40
        .BYTE $1A,$60,$22,$21,$10,$31,$13,$40
        .BYTE $11,$13,$21,$20,$11,$22,$60,$13,$40
        .BYTE $10,$13,$50,$22,$10,$1A,$40,$13,$40
        .BYTE $10,$13,$30,$1A,$10,$22,$31,$30,$13,$40
        .BYTE $10,$13,$20,$31,$22,$60,$13,$40
        .BYTE $10,$13,$1A,$40,$22,$20,$1A,$30,$13,$40
        .BYTE $31,$40,$61,$20,$13,$40
        .BYTE $1A,$E0,$13,$40
        .BYTE $F1,$11,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50

level11:.BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $50,$41,$b0
        .BYTE $11,$e0,$50
        .BYTE $20,$1A,$10,$1A,$20,$1A,$10,$1A,$10,$22,$30,$40
        .BYTE $20,$21,$13,$20,$13,$21,$10,$22,$20,$1A,$40
        .BYTE $40,$13,$20,$13,$20,$1a,$22,$10,$21,$40
        .BYTE $40,$13,$2A,$13,$20,$11,$22,$1a,$20,$40
        .BYTE $11,$30,$11,$23,$11,$30,$22,$11,$20,$40
        .BYTE $20,$1A,$20,$23,$40,$22,$2a,$10,$40
        .BYTE $F1,$11,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50

level12:.BYTE $F0,$50
        .BYTE $10,$1A,$10,$1A,$30,$1A,$10,$1A,$60,$40
        .BYTE $21,$13,$11,$30,$11,$13,$11,$20,$1A,$10,$1A,$10,$40
        .BYTE $20,$13,$50,$13,$30,$41,$40
        .BYTE $20,$13,$40,$1A,$13,$70,$40
        .BYTE $30,$14,$25,$16,$21,$70,$40
        .BYTE $20,$13,$D0,$40
        .BYTE $10,$13,$11,$50,$11,$13,$10,$17,$28,$19,$10,$40
        .BYTE $11,$13,$70,$13,$60,$40
        .BYTE $1A,$13,$1A,$50,$1A,$13,$40,$2A,$40
        .BYTE $31,$50,$81,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50

level13:.BYTE $90,$12,$60,$40
        .BYTE $11,$10,$1A,$60,$12,$40,$1a,$10,$40
        .BYTE $10,$21,$20,$1A,$30,$12,$20,$1A,$10,$21,$40
        .BYTE $40,$41,$13,$21,$13,$11,$30,$40
        .BYTE $80,$13,$20,$13,$40,$40
        .BYTE $80,$13,$2A,$13,$40,$40
        .BYTE $70,$13,$41,$13,$30,$40
        .BYTE $10,$2A,$40,$13,$40,$13,$30,$40
        .BYTE $14,$15,$16,$40,$13,$40,$13,$20,$11,$40
        .BYTE $50,$2A,$13,$40,$13,$2A,$10,$40
        .BYTE $10,$31,$10,$31,$40,$41,$40
        .BYTE $F0,$50
        .byte $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50

level14:.BYTE $F0,$50
        .BYTE $90,$1A,$60,$40
        .BYTE $1A,$30,$1A,$10,$1A,$20,$11,$13,$30,$1A,$10,$40
        .BYTE $11,$20,$21,$13,$11,$30,$13,$20,$31,$40
        .BYTE $50,$13,$40,$13,$50,$40
        .BYTE $40,$1A,$13,$40,$1A,$50,$40
        .BYTE $40,$21,$20,$31,$10,$17,$18,$19,$13,$40
        .BYTE $90,$13,$50,$13,$40
        .BYTE $11,$13,$70,$13,$50,$13,$40
        .BYTE $10,$13,$20,$1A,$20,$1A,$30,$1A,$20,$1A,$13,$40
        .BYTE $10,$51,$10,$31,$10,$21,$10,$21,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50

level15:.BYTE $F0,$50
        .BYTE $A0,$1A,$50,$40
        .BYTE $1A,$30,$1A,$50,$11,$50,$40
        .BYTE $21,$10,$21,$10,$21,$13,$70,$40
        .BYTE $80,$13,$70,$40
        .BYTE $80,$13,$1A,$40,$11,$13,$40
        .BYTE $80,$21,$10,$31,$10,$13,$40
        .BYTE $F0,$13,$40
        .BYTE $11,$E0,$13,$40
        .BYTE $1A,$60,$1A,$40,$1A,$10,$1A,$13,$40
        .BYTE $42,$20,$42,$10,$52,$40
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $F0,$50
        .BYTE $21,$10,$21,$50,$21,$40,$40

;0=nothing
;1=jump                    ok
;2=throw spanner           ok
;3=hit by spanner/bird     ok
;4=death                   ok
;5=eat lunch               ok
;6=levelend Score update   ok

sound:
	cpx #4
	bne chk3
	lda #10
	sta TMP
sndloop: 
	lda #2
	ldx TMP
	ldy #2
	jsr playsound
	inc TMP
	lda TMP
	cmp #80
	bne sndloop
	lda #20
	ldx #32
	ldy #32
	jsr playsound
	rts	
chk3:
;	cpx #3
;	bne makesound
;	lda #10
;	sta TMP
;sndloop2: 
;	lda #2
;	ldx TMP
;	ldy #16
;	jsr playsound
;	inc TMP
;	lda TMP
;	cmp #80
;	bne sndloop2
;	rts	

makesound:

	ldy soundoctave,x
	lda soundlen,x
	pha
	lda soundpitch,x
	tax
	pla
	jsr playsound
	rts
 
;             100,6,6,200,130,5
soundpitch:
	.db  1,16,32,32, 1,16,16
soundoctave:
	.db  1, 2, 4,32, 1,32, 4
soundlen:
	.db 10,10,10,70,10,10, 2
                                         
;--------------------------------
;Sprite types -
;
; 0 - Big Kong
;     Stands still, throws barrels
;     Also plots damsel sprite
;     Size 12 x 32
;     Sprites 0-1 for Kong, 2 for haylp, 3 for barrel. Damsel sprite with players
;     Uses seperate variables, so no more than one per screen!
;
; 1 - Bird
;     Moves between D1 and D2
;     Sprites 4-9
;     Size 12 x 12
;
; 2 - Monkey
;     Sits still and throws stuff
;     Sprites 10 & 11
;     Size 12 x 16
;
; 3 - Peter the Panda
;     Sits still and does nothing
;     Size 12 x 32
;     Sprite 16
;
;Sprite format -
;
; Byte 0 - X and D1, or $FF for end of list
; Byte 1 - bits 7-3 = Y
;          bits 2-0 = type
; Byte 2 - bit 7 - direction
;          bits 6-0 - D2
;--------------------------------

sprites1: .BYTE 40,0,0
          .BYTE 0+128,40+1,36
          .BYTE 8,96+1,24
          .BYTE 28,56+1,56
          .BYTE $FF

sprites2: .BYTE 52,112+2,$80
          .BYTE 48,64+2,$80
          .BYTE 48,16+2,$80
          .BYTE 12,112+1,48          
          .BYTE $FF          

sprites3: .BYTE 0,8+1,56
          .BYTE 44,64+2,$80
          .BYTE 0,144+2,0
          .BYTE 8,96+1,48
          .BYTE $FF

sprites4: .BYTE 40,144+2,$80
          .BYTE 12,8+1,50
          .BYTE 54,96+1,12+128
          .BYTE 24,112+2,0
          .BYTE $FF
          
sprites5: .BYTE 24,0,0
          .BYTE 20+128,48+1,52
          .BYTE 20+128,192+1,48
          .BYTE $FF

sprites6: .BYTE 0,8+1,24
          .BYTE 60,8+1,36+128
          .BYTE 16,80+1,40
          .BYTE 12,120+1,44
          .BYTE $FF

sprites7: .BYTE 0,8+1,44
          .BYTE 52,48+1,12+128
          .BYTE 4,96+1,52
          .BYTE 48,64+2,$80
          .BYTE $FF

sprites8: .BYTE 0,64+2,60
          .BYTE 60,0+2,$80
          .BYTE 0,8+1,44
          .BYTE 4,120+1,52          
          .BYTE $FF

sprites9: .BYTE 0,8+1,56
          .BYTE 60,64+2,$80
          .BYTE 60,144+2,$80
          .BYTE $FF
          
sprites10: .BYTE 36,16+2,0
           .BYTE 36,64+2,0
           .BYTE 36,112+2,0
           .BYTE 24,32+2,$80
           .BYTE 24,80+2,$80
           .BYTE 8,112+1,32                    
           .BYTE $FF

sprites11: .BYTE 22,0,0
           .BYTE 0,32+2,0
           .BYTE 60,144+2,$80
           .BYTE 64,144+2,0                
           .BYTE $FF

sprites12: .BYTE 48,128+3,0
           .BYTE 0,16+2,0
           .BYTE 8,128+1,28
           .BYTE 36,64+1,56
           .BYTE $FF

sprites13: .BYTE 0,0+2,0
           .BYTE 60,112+2,$80
           .BYTE 40,8+1,60
           .BYTE $FF

sprites14: .BYTE 0,8+1,56
           .BYTE 8,144+1,52
           .BYTE 32,80+2,0
           .BYTE 60,32+2,$80           
           .BYTE $FF

sprites15: .BYTE 0,144+1,28      ;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
           .BYTE 52,144+1,32+128
           .BYTE 56,8+1,$80
           .BYTE 36,48+1,56
           .BYTE $FF

sprites16: .BYTE 26,0,0
           .BYTE 56,40+1,40+128
           .BYTE 16,88+1,40
           .BYTE 0,40+1,16
           .BYTE $FF

;--------------------------------
;Data tables
;--------------------------------

levs:	   .word level2,  level13,  level3,  level4,  level6,  level12,  level14,  level5,  level9,  level6,   level7,  level8,  level11,  level10,  level15,  level1
sprs:      .word sprites2,sprites13,sprites3,sprites4,sprites6,sprites12,sprites14,sprites5,sprites9,sprites16,sprites7,sprites8,sprites11,sprites10,sprites15,sprites1
sprrouts:  .word bkrout,birdrout,monkeyrout,pandarout
spr24:     .word gr1,gr2,haylp,br1,bd11,bd12,bd13,bd21,bd22,bd23,mn1,mn2,sp1,sp2,sp3,sp4,pn1
frames:    .db 0,1,0,2
solid:     .db 0,1,1,0,1,1,1,1,1,1,0

;--------------------------------
;String declarations
;--------------------------------

banner:    .db blue+5, 1,'*** HARD HAT HARRY ***',0,$ff
hiscr:     .db yellow+11, 4,'HIGH SCORES',0,$ff   
hiline:    .db red+7, 5,' 1)  HHH  00100000',0,$ff
rs:        .db blue+6,19,'RETRO SOFTWARE  2011',0,$ff
prsspc:    .db red+2,22,'PRESS SPACE OR FIRE TO START',0,$ff
levelstr:  .db blue+12,12,'LEVEL',0,$FF
prtscore:
prtnum:	   .db blue+0, 0,'00000000',0,$ff
livesstr:  .db blue+25, 0,'LIVES=',0,$FF        
timestr:   .db blue+24,23,'TIME=',0
timenum    .db blue+29,23,'999',0,$FF
level2str: .db blue+0,23,'LEVEL=',0,$FF
gameovr:   .db blue+11,12,"GAME  OVER",0,$FF
typename:  .db yellow+7,19,'CONGRATULATIONS!!!',0
           .db blue+8,22,'ENTER YOUR NAME',0,$ff
