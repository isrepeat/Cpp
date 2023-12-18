# ----------------------------
#          Building           
# ----------------------------
# --- Get the GCC preinstalled image from Docker Hub --- 
FROM gcc:latest as build_layer

# --- Install neccessary packages (Ubuntu) ---
RUN apt-get update                                                                                \
    &&  apt-get install -y                                                                        \
          protobuf-compiler                                                                       \
          libssl-dev                                                                              \
          cmake                                                                                   \
          make                                                                                    \
          gcc                                                                                     \
          git																		 

# --- Copy current dir to container and build ---
COPY . /STUNServer
WORKDIR ./STUNServer

# --- Install p7 ---
RUN cd 3rdParty                                                                                   \
    &&  mkdir p7                                                                                  \
    &&  cd p7                                                                                     \
    &&  curl -LO http://baical.net/files/libP7Client_v5.6.zip                                     \
    &&  unzip libP7Client_v5.6.zip -d .                                                           \
    &&  mkdir bin                                                                                 \
    &&  cd bin                                                                                    \
    &&  cmake ..                                                                                  \
    &&  make                                                                                      \
    &&  make install                                                                              \
    # return to Dockerfile directory
    &&  cd ../../../

# --- Install msquic ---
RUN cd 3rdParty                                                                                   \
    &&  git clone --depth 1 --branch v1.2.0 https://github.com/microsoft/msquic.git               \
    &&  cd msquic                                                                                 \
    # NOTE: Ensure link for current tag is correct
    &&  curl -LO https://github.com/microsoft/msquic/releases/download/v1.2.0/msquic_linux_x64_Release_openssl.zip \
    &&  unzip msquic_linux_x64_Release_openssl.zip -d ./release_1.2.0                             \
    # return to Dockerfile directory
    &&  cd ../../


# --- Install neccessary packages (CentOs) ---
#RUN apt-get update                                           \
#    &&  apt-get install -y                                   \
#          openssl-devel                                      \
#          libatomic                                          \
#          gcc-c++                                            \
#          cmake                                              \
#          git    


WORKDIR ./STUNServer/Release

RUN cmake .. && cmake --build .

ENTRYPOINT [".bin/STUNServer"]