.global outb
.global inb

# outb - Send a byte to an I/O port
# stack: [esp + 8] Byte data
#        [esp + 4] Target I/O port.
#        [esp    ] return address
outb:
    mov 8(%esp), %al
    mov 4(%esp), %dx
    out %al, %dx
    ret

# inb - returns a byte from the given I/O port
# stack: [esp + 4] The address of the I/O port
#        [esp    ] The return address
inb:
    mov 4(%esp), %dx
    in %dx, %al
    ret
