# ----------------------------
#        Build layer          
# ----------------------------
# --- Get the GCC preinstalled image from Docker Hub --- 
FROM gcc:latest as build_layer

ARG HOST_DISTRIBUTION=ubuntu

# --- Install neccessary packages ---
RUN if [ "HOST_DISTRIBUTION" = "ubuntu" ]; then                                                      \
       apt-get update                                                                                \
       &&  apt-get install -y                                                                        \
             protobuf-compiler                                                                       \
             libssl-dev                                                                              \
             cmake                                                                                   \
             make                                                                                    \
             gcc                                                                                     \
             git                                                                                     \
    elif [ "HOST_DISTRIBUTION" = "centos" ]; then                                                    \  
       dnf check-update                                                                              \
       &&  dnf install -y                                                                            \
             protobuf-compiler                                                                       \
             openssl-devel                                                                           \
             libatomic                                                                               \
             gcc-c++                                                                                 \
             cmake                                                                                   \
             make                                                                                    \
             git                                                                                     \
    fi

# --- Copy current dir to container and build ---
COPY . ./STUNServer
WORKDIR ./STUNServer

# --- Install p7 ---
RUN cd 3rdParty                                                                                      \
    &&  mkdir p7                                                                                     \
    &&  cd p7                                                                                        \
    &&  curl -LO http://baical.net/files/libP7Client_v5.6.zip                                        \
    &&  unzip libP7Client_v5.6.zip -d .                                                              \
    &&  mkdir bin                                                                                    \
    &&  cd bin                                                                                       \
    &&  cmake ..                                                                                     \
    &&  make                                                                                         \
    &&  make install                                                                                 \
    # back to Dockerfile directory
    &&  cd ../../../

# --- Install msquic ---
RUN cd 3rdParty                                                                                      \
    &&  git clone --depth 1 --branch v1.2.0 https://github.com/microsoft/msquic.git                  \
    &&  cd msquic                                                                                    \
    # NOTE: Ensure link for current tag is correct
    &&  curl -LO https://github.com/microsoft/msquic/releases/download/v1.2.0/msquic_linux_x64_Release_openssl.zip \
    &&  unzip msquic_linux_x64_Release_openssl.zip -d ./release_1.2.0                                \
    # back to Dockerfile directory
    &&  cd ../../

WORKDIR ./Release
RUN cmake .. && cmake --build .


# TODO: Add prodaction layer and copy executabe & deps from build_layer
WORKDIR ./bin
ENTRYPOINT ["./STUNServer"]