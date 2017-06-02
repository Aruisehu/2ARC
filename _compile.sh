ca65 crt0.s

cc65 -Oi utils.c --add-source

ca65 utils.s

cc65 -Oi $1.c --add-source

ca65 $1.s

ld65 -C $2.cfg -o $1.nes crt0.o $1.o utils.o runtime.lib

rm $1.s

