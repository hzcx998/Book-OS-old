[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_FS_LOAD_INODE EQU 70
_NR_FS_DRIVE_CONNECT EQU	71
_NR_FS_DRIVE_GET	 EQU	72

global fs_load_inode
fs_load_inode:	; void fs_load_inode(struct inode_s *buf, unsigned int id);
	mov eax, _NR_FS_LOAD_INODE
	mov ebx, [esp + 4]		
	mov ecx, [esp + 8]		
	int INT_VECTOR_SYS_CALL
	ret

global fs_drive_connect
fs_drive_connect:	; struct drive_s *fs_drive_connect();
	mov eax, _NR_FS_DRIVE_CONNECT	
	int INT_VECTOR_SYS_CALL
	ret

global fs_drive_get
fs_drive_get:	; struct drive_s *fs_drive_get(struct drive_s *buf);
	mov eax, _NR_FS_DRIVE_GET
	mov ebx, [esp + 4]		
	int INT_VECTOR_SYS_CALL
	ret


