# ----------------------------
#          Building           
# ----------------------------
# --- Get the GCC preinstalled image from Docker Hub --- 
FROM gcc:latest as build_layer

# --- Install neccessary packages (Ubuntu) ---
RUN apt-get update                                           \
    &&  apt-get install -y                                   \
          protobuf-compiler                                  \
          libssl-dev                                         \
          cmake                                              \
          gcc                                                \
          git                                                



# --- Install neccessary packages (CentOs) ---
#RUN apt-get update                                           \
#    &&  apt-get install -y                                   \
#          openssl-devel                                      \
#          libatomic                                          \
#          gcc-c++                                            \
#          cmake                                              \
#          git    

# --- Copy sources to container ---
COPY ./src /app/src
COPY ./CMakeLists.txt /app/CMakeLists.txt

WORKDIR /app/build

# --- Build ---
RUN cmake -DCMAKE_BUILD_TYPE=Release /app                    \
    &&  cmake --build .


ENTRYPOINT ["./HelloWorld"]