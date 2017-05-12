global set_vga_mode
set_vga_mode:
  mov ah, [esp + 8]    ; param 2 : vga mode set
  mov al, [esp + 4]    ; param 1 : vga mode
  int 0x10
  ret
