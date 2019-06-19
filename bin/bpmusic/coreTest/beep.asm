extern main
extern exit
[bits 32]

GLOBAL _api_beep														;蜂鸣发声API接口

[section .text]

;蜂鸣发声器相关====================================================
_api_beep:							;void api_beep(int tone);
		MOV		EDX,20
		MOV		EAX,[ESP+4]			; tone
		INT		0x40
		RET
		