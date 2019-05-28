### Makefile ---

## Author: aign
## Keywords:

SOLVER = glucose30i

obj:
	@-(cd $(SOLVER)/core; make obj)
	@-(cd $(SOLVER)/utils; make obj)
	@-(cd $(SOLVER)/simp; make obj)
	@-mv $(SOLVER)/core/*.o .
	@-mv $(SOLVER)/utils/*.o .
	@-mv $(SOLVER)/simp/*.o .

lib:
	@-(cd $(SOLVER)/core; make obj)
	@-(cd $(SOLVER)/utils; make obj)
	@-(cd $(SOLVER)/simp; make obj)
	@-mkdir bolt_arch
	@-mv $(SOLVER)/core/*.o bolt_arch
	@-mv $(SOLVER)/utils/*.o bolt_arch
	@-mv $(SOLVER)/simp/*.o bolt_arch
	@-(cd bolt_arch; ar cr libsat.a *.o; mv libsat.a ../.; rm *.o)
	@-rmdir bolt_arch

clean:
	@-rm -f libsat.a
	@-rm -f *.o

### Makefile ends here
