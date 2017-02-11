obj = mdprincipal.o mdfonctions.o mddatatypes.o mdreqtools.o mdappicon.o
source = mdprincipal.c mdfonctions.c mddatatypes.c mdreqtools.c mdappicon.c
linkflags = -noixemul
flags =
# -DDEBUG

MD : $(obj)
	gcc $(linkflags) -o MD $(obj) -lreqtools

mdprincipal.o : mdprincipal.c
	gcc -c mdprincipal.c $(flags)

mdfonctions.o : mdfonctions.c
	gcc -c mdfonctions.c $(flags)

mddatatypes.o : mddatatypes.c
	gcc -c mddatatypes.c $(flags)

mdreqtools.o : mdreqtools.c
	gcc -c mdreqtools.c $(flags)

mdappicon.o : mdappicon.c
	gcc -c mdappicon.c $(flags)

clear :
	rm *.o
