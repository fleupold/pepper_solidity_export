CXX = g++

# Make sure these are the same flags that pepper was built with
CXXFLAGS = -m64 -std=c++11 -DCURVE_ALT_BN128 -DBN_SUPPORT_SNARK -DBINARY_OUTPUT -DMONTGOMERY_OUTPUT -DNO_PROCPS -DUSE_ASM

IFLAGS = -I$(LIBSNARK)/ -I$(LIBSNARK)/depends/libff -I$(LIBSNARK)/depends/libfqfft
LDFLAGS = -L$(LIBSNARK)/build/libsnark -L$(LIBSNARK)/build/depends/libff/libff
LDFLAGS += -lsnark -lff -lgmp

BINDIR = bin

export: export.cpp check-env
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(IFLAGS) $< -o $(BINDIR)/$@ $(LDFLAGS)

check-env:
ifndef LIBSNARK
  $(error $$LIBSNARK is undefined)
endif

clean:
	rm -rf $(BINDIR)
