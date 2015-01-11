# objects list
SOURCE_CODE_PATH   = ./src
MODULE_PATH        = ./output/binary
OBJS_PATH          = ./output/objects
MODULE_FILE        = $(MODULE_PATH)/module
SCRIPT_FILE        = $(SOURCE_CODE_PATH)/linker-script.ld
OBJS               = $(OBJS_PATH)/startup.o $(OBJS_PATH)/main.o

# flags
CCFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-asynchronous-unwind-tables -c -m32
ASFLAGS = -f aout
LDFLAGS = --script $(SCRIPT_FILE)

$(MODULE_FILE) : $(OBJS)
	ld $(OBJS) $(LDFLAGS) -o $@

$(OBJS_PATH)/%.o : $(SOURCE_CODE_PATH)/%.s
	nasm $< $(ASFLAGS) -o $@

$(OBJS_PATH)/%.o : $(SOURCE_CODE_PATH)/%.c
	gcc $< $(CCFLAGS)  -o $@

clean:
	rm -f $(OBJS) $(MODULE_FILE)

