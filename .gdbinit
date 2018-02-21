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

define sss
    x/64wx $esp
end

set disassembly-flavor intel
target remote localhost:1234
symbol-file out/myos.bin
break Debugger
tui enable
continue
