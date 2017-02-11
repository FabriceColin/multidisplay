ccomp = gcc
catcomp = catcomp
linkflags = -noixemul -lreqtools
flags = -Wall -g -DDEBUG
bin = MultiDisplay
obj = mdprincipal.o mdfonctions.o mddatatypes.o mdreqtools.o mdappicon.o
cat = MultiDisplay.catalog

menu:
	@echo "Usage is:"
	@echo "make binary, to build the main executable"
	@echo "make locale, to build the language strings file and the localized catalogs"
	@echo "make clear, to delete all object files"

binary : $(obj)
	$(ccomp) $(flags) -o $(bin) $(obj) $(linkflags)

locale :
	$(catcomp) Catalogs/Translations/md.cd NOCODE CFILE=mdstrings.h
	$(catcomp) Catalogs/Translations/md.cd Catalogs/Translations/english.ct CATALOG=Catalogs/English/$(cat)

mdprincipal.o : mdprincipal.c
	$(ccomp) -c mdprincipal.c $(flags)

mdfonctions.o : mdfonctions.c
	$(ccomp) -c mdfonctions.c $(flags)

mddatatypes.o : mddatatypes.c
	$(ccomp) -c mddatatypes.c $(flags)

mdreqtools.o : mdreqtools.c
	$(ccomp) -c mdreqtools.c $(flags)

mdappicon.o : mdappicon.c
	$(ccomp) -c mdappicon.c $(flags)

clear :
	@delete $(obj)
