all: fcgen fcbt

fcgen: fcgen.cc

fcbt: fcbt.cc
	g++ -g -o fcbt fcbt.cc

clean:
	rm -f fcgen fcgen.exe fcbt fcbt.exe *.stackdump
