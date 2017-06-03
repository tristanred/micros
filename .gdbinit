define jumpd
    tbreak $arg0
    jump $arg0
end

target remote localhost:1234
symbol-file out/myos.bin
break kernel_main
break Debugger
tui enable
continue
