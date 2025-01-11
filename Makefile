TESTS:= \
	scu-dma

ifeq ($(strip $(YAUL_INSTALL_ROOT)),)
  $(error Undefined YAUL_INSTALL_ROOT (install root directory))
endif

ifeq ($(strip $(SILENT)),)
  ECHO=
else
  ECHO=@
endif
export ECHO

.PHONY: all clean list-tests

all clean:
	$(ECHO)for test in $(TESTS); do \
		printf -- "[1;36m$@[m [1;32mtests/$$test[m\n"; \
		($(MAKE) -C $$test $@) || exit $$?; \
	done

# Exclude directories: shared
#                      _template
list-tests:
	$(ECHO)/usr/bin/find . -maxdepth 1 -type d | tail -n +2 | sed -E 's/^\.\///g;/^\./d;/^_/d;/^shared$$/d' | sort -n
