define jumpd
    tbreak $arg0
    jump $arg0
end

define killq
    kill
    q
end

define bc
    tb $arg0
    c
end

define ln
    layout next
end

target remote localhost:1234
symbol-file out/myos.bin
break Debugger
tui enable
layout split
focus src
continue
