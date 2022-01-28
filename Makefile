.PHONY: build
build:
	bash -c 'cd edk2 && source ./edksetup.sh && build'

.PHONY: run
run:
	mkdir -p ./image/EFI/BOOT
	cp ./edk2/Build/MitnalX64/DEBUG_CLANG38/X64/Mitnal.efi ./image/EFI/BOOT/BOOTX64.efi
	qemu-system-x86_64 \
		-drive if=pflash,format=raw,readonly=on,file=./OVMF_CODE.fd \
		-drive if=pflash,format=raw,file=./OVMF_VARS.fd \
		-drive if=ide,file=fat:rw:image,index=0,media=disk