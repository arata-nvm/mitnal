FROM --platform=linux/amd64 ubuntu:20.04

RUN apt update \
  && apt install -y build-essential git uuid-dev python3-distutils clang llvm nasm acpica-tools p11-kit

RUN git clone --recursive https://github.com/tianocore/edk2.git \
  && cd edk2 \
  && git checkout stable/202011 \
  && make -C BaseTools/Source/C

RUN sed -i 's/DEFINE NETWORK_TLS_ENABLE             = FALSE/DEFINE NETWORK_TLS_ENABLE             = TRUE/' edk2/OvmfPkg/OvmfPkgX64.dsc \
  && sed -i 's/DEFINE NETWORK_HTTP_BOOT_ENABLE       = FALSE/DEFINE NETWORK_HTTP_BOOT_ENABLE       = TRUE/' edk2/OvmfPkg/OvmfPkgX64.dsc

RUN cd edk2 && bash -c 'source edksetup.sh && build -a X64 -b RELEASE -t CLANG38 -p OvmfPkg/OvmfPkgX64.dsc'

COPY MitnalPkg edk2/MitnalPkg

RUN git clone https://github.com/Akagi201/hmac-sha1 \
  && mv hmac-sha1/src/sha edk2/MitnalPkg/ \
  && mv hmac-sha1/src/hmac edk2/MitnalPkg/ \
  && sed -i '11i #include <stddef.h>\n' edk2/MitnalPkg/hmac/hmac.h 

RUN git clone https://github.com/sheredom/json.h \
  && mv json.h/json.h edk2/MitnalPkg/ \
  && sed -i '/#include <string.h>/d' edk2/MitnalPkg/json.h \
  && sed -i '/#include <stdlib.h>/d' edk2/MitnalPkg/json.h 

RUN git clone https://github.com/tanakamasayuki/efont \
  && mv efont/src edk2/MitnalPkg/font \
  && sed -i '/#include <Arduino.h>/d' edk2/MitnalPkg/font/efont.h

RUN cd edk2 && bash -c 'source edksetup.sh && build -a X64 -b RELEASE -t CLANG38 -p MitnalPkg/MitnalPkg.dsc'

RUN p11-kit extract --format=edk2-cacerts --filter=ca-anchors --purpose=server-auth certdb
