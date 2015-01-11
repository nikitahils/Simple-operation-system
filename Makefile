# objects list
#
CODE_PATH          = ./src
MODULE_PATH        = ./module/output/binary
OUTPUT_BINARY_PATH = ./output/kernel
OUTPUT_LINKER_PATH = ./output/objects
OUTPUT_IMAGES_PATH = ./output/images
KERNEL_FILE        = $(OUTPUT_BINARY_PATH)/kernel
MODULE_FILE        = $(MODULE_PATH)/module
DISK_IMAGE_FILE    = $(OUTPUT_IMAGES_PATH)/hdd.img
OBJS               = $(OUTPUT_LINKER_PATH)/boot.o $(OUTPUT_LINKER_PATH)/loader.o $(OUTPUT_LINKER_PATH)/start_kernel.o        \
                     $(OUTPUT_LINKER_PATH)/common.o $(OUTPUT_LINKER_PATH)/screen.o $(OUTPUT_LINKER_PATH)/descriptor_tables.o \
                     $(OUTPUT_LINKER_PATH)/interrupt.o $(OUTPUT_LINKER_PATH)/isr.o $(OUTPUT_LINKER_PATH)/timer.o             \
                     $(OUTPUT_LINKER_PATH)/paging.o $(OUTPUT_LINKER_PATH)/kheap.o  $(OUTPUT_LINKER_PATH)/kbitmap.o           \
		     $(OUTPUT_LINKER_PATH)/panic.o $(OUTPUT_LINKER_PATH)/rtc.o $(OUTPUT_LINKER_PATH)/keyboard.o              \
		     $(OUTPUT_LINKER_PATH)/mutex.o $(OUTPUT_LINKER_PATH)/memory_manager.o $(OUTPUT_LINKER_PATH)/alloc.o      \
		     $(OUTPUT_LINKER_PATH)/syscall.o  $(OUTPUT_LINKER_PATH)/module_loader.o $(OUTPUT_LINKER_PATH)/module.o   \
	             $(OUTPUT_LINKER_PATH)/kterminal.o
# flags
CCFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-asynchronous-unwind-tables -c -m32 -ggdb3
ASFLAGS = -f aout
LDFLAGS = -m elf_i386 --script $(CODE_PATH)/linker-script.ld --no-check-sections

usb_flash: $(DISK_IMAGE_FILE)
	dd if=$< of=/dev/sdb

debug: $(DISK_IMAGE_FILE)
	qemu-system-i386 -s -S -hda $< &
	cgdb $(KERNEL_FILE).dbg

compile: $(DISK_IMAGE_FILE)

image: $(DISK_IMAGE_FILE)

all: $(KERNEL_FILE) $(DISK_IMAGE_FILE) 
	qemu-system-i386 -hda $(DISK_IMAGE_FILE)

$(DISK_IMAGE_FILE): $(KERNEL_FILE)
	@echo "Creating hdd.img..."
	@dd if=/dev/zero of=$@ bs=512 count=16065 1>/dev/null 2>&1	
	@echo "Creating bootable first FAT32 partition..."
	@losetup /dev/loop1 $@
	@(echo c; echo u; echo n; echo p; echo 1; echo ;  echo ; echo a; echo 1; echo t; echo c; echo w;) | fdisk /dev/loop1 1>/dev/null 2>&1 || true

	@echo "Mounting partition to /dev/loop2..."
	@losetup /dev/loop2 $@ \
	--offset    `echo \`fdisk -lu /dev/loop1 | sed -n 10p | awk '{print $$3}'\`*512 | bc` \
	--sizelimit `echo \`fdisk -lu /dev/loop1 | sed -n 10p | awk '{print $$4}'\`*512 | bc`
	@losetup -d /dev/loop1

	@echo "Format partition..."
	@mkdosfs -F32 /dev/loop2

	@echo "Copy kernel and grub files on partition..."
	@mkdir -p tempdir
	@mount /dev/loop2 tempdir
	@mkdir tempdir/boot
	@sudo cp -r grub tempdir/boot/
	@cp $< tempdir/
	@cp $(MODULE_FILE) tempdir/ || echo try\ to\ copy\ module...
	@sleep 1
	@umount /dev/loop2
	@rm -r tempdir
	@losetup -d /dev/loop2

	@echo "Installing GRUB..."
	@echo "device (hd0) $@ \n \
		root (hd0,0)         \n \
		setup (hd0)          \n \
		quit\n" | grub --batch 1>/dev/null
	@echo "Done!"

$(KERNEL_FILE) : $(OBJS)
	ld $(OBJS) $(LDFLAGS) -o $@
	cp $@ $@.dbg
	strip -s $@

$(OUTPUT_LINKER_PATH)/%.o : $(CODE_PATH)/%.s
	nasm $< $(ASFLAGS) -o $@

$(OUTPUT_LINKER_PATH)/%.o : $(CODE_PATH)/%.c
	gcc $< $(CCFLAGS)  -o $@

clean:
	rm -f $(OBJS) $(KERNEL_FILE) $(KERNEL_FILE).dbg $(DISK_IMAGE_FILE)

