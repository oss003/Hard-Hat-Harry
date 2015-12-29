

codebase     equ $3000

               org codebase-22

;****************************************
; Header for Atom emulator Wouter Ras
		 .db "HARRY           "
		 .dw codebase
		 .dw codebase
		 .dw eind_asm-start_asm
;****************************************
start_asm    
               include harry.inc
	       include sound.inc
               include sprites.inc
               include tiles.inc
               include font.inc

eind_asm
