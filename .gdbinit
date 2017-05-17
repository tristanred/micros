target remote localhost:1234
symbol-file out/myos.bin
break kernel_main
break Debugger
tui enable
continue
