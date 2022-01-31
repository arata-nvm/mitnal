.PHONY: build
build:
	bash -c 'cd edk2 && source ./edksetup.sh && build'
	mkdir -p dist
	cp ./edk2/Build/OvmfX64/DEBUG_CLANG38/FV/OVMF.fd ./dist/
	cp ./edk2/Build/MitnalX64/DEBUG_CLANG38/X64/Mitnal.efi ./dist/

.PHONY: run
run:
	mkdir -p ./image/EFI/BOOT
	cp ./dist/Mitnal.efi ./image/EFI/BOOT/BOOTX64.efi
	qemu-system-x86_64 \
		-bios ./dist/OVMF.fd \
		-drive if=ide,format=raw,file=fat:rw:image,index=0,media=disk \
		-fw_cfg name=etc/edk2/https/cacerts,file=./dist/certdb

.PHONY: docker/build
docker/build:
	mkdir -p dist
	docker build -t arata-nvm/mitnal .
	docker container create --name mitnal-temp arata-nvm/mitnal
	docker cp mitnal-temp:/edk2/Build/OvmfX64/RELEASE_CLANG38/FV/OVMF.fd ./dist/
	docker cp mitnal-temp:/edk2/Build/MitnalX64/RELEASE_CLANG38/X64/Mitnal.efi ./dist/
	docker cp mitnal-temp:/certdb ./dist/
	docker container rm mitnal-temp

