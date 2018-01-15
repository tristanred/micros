global tswitch

extern test_args

tswitch:
    pusha
    call test_args
    popa
    ret
