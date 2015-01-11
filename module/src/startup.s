[BITS 32]
MAGIC  equ 0xDEADBEEF 

[EXTERN code_offset]              
[EXTERN code_size]                  
[EXTERN data_offset]              
[EXTERN data_size]   
[EXTERN main]  
[GLOBAL _start]                  
                
    dd  MAGIC         
    dd  code_offset                    
    dd  code_size                     
    dd  data_offset 
    dd  data_size                   
    dd  _start                                

_start:
    push eax
    push ebx
    call main  
    int 0x0
    mov eax, 0xFFFFFFFF 
    mov edx, [eax]
