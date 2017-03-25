.global outb

# outb - Send a byte to an I/O port
# stack: [esp + 8] Byte data
#        [esp + 4] Target I/O port.
#        [esp    ] return address
outb:
    mov 8(%esp), %al
    mov 4(%esp), %dx
    out %al, %dx
    ret
