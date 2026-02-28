CC     = clang
TARGET = wasm32
OUTDIR = ../public
CFLAGS = --target=$(TARGET) -nostdlib -O2 \
         -Wl,--no-entry -Wl,--export-dynamic

all: $(OUTDIR)/sim.wasm

$(OUTDIR)/sim.wasm: sim.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(OUTDIR)/sim.wasm

.PHONY: all clean
