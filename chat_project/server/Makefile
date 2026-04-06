QMAKE ?= qmake
MAKE ?= mingw32-make
GEN_MAKEFILE = Makefile.generated

all:
	$(QMAKE) server.pro -o $(GEN_MAKEFILE)
	$(MAKE) -f $(GEN_MAKEFILE)

clean:
	@if exist $(GEN_MAKEFILE) $(MAKE) -f $(GEN_MAKEFILE) clean
	@if exist release rmdir /s /q release
	@if exist debug rmdir /s /q debug
	@if exist chat_server.exe del /q chat_server.exe
	@if exist .qmake.stash del /q .qmake.stash
