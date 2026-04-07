QMAKE ?= qmake
MAKE ?= mingw32-make
GEN_MAKEFILE = Makefile.generated

all:
	$(QMAKE) web_gateway.pro -o $(GEN_MAKEFILE)
	$(MAKE) -f $(GEN_MAKEFILE)

clean:
	@if exist $(GEN_MAKEFILE) $(MAKE) -f $(GEN_MAKEFILE) clean
	@if exist release rmdir /s /q release
	@if exist debug rmdir /s /q debug
	@if exist chat_web_gateway.exe del /q chat_web_gateway.exe
	@if exist .qmake.stash del /q .qmake.stash
