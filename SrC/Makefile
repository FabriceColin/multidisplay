ccomp = tovbr vc
catcomp = catcomp
linkflags = -lreqtools
flags = +warpos -amiga-align
# -DDEBUG_INFO
bin = MultiDisplay
obj = mdprincipal.o mdfonctions.o mddatatypes.o mdreqtools.o mdappicon.o
cat = MultiDisplay.catalog

menu:
	@echo "Usage is:"
	@echo "make binary, to build the main executable"
	@echo "make locale, to build the language strings file and the localized catalogs"
	@echo "make clear, to delete all object files and the executable"

binary : $(obj)
	$(ccomp) $(flags) -o $(bin) $(obj) $(linkflags)

locale :
	$(catcomp) Catalogs/Translations/multidisplay.cd NOCODE CFILE=mdstrings.h
	$(catcomp) Catalogs/Translations/multidisplay.cd Catalogs/Translations/english.ct CATALOG=Catalogs/English/$(cat)

mdprincipal.o : mdprincipal.c
	$(ccomp)  $(flags) -c mdprincipal.c

mdfonctions.o : mdfonctions.c
	$(ccomp)  $(flags) -c mdfonctions.c

mddatatypes.o : mddatatypes.c
	$(ccomp)  $(flags) -c mddatatypes.c

mdreqtools.o : mdreqtools.c
	$(ccomp)  $(flags) -c mdreqtools.c

mdappicon.o : mdappicon.c
	$(ccomp)  $(flags) -c mdappicon.c

clear :
	@delete $(obj)
