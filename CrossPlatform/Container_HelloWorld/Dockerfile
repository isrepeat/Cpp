# ----------------------------
#          Building           
# ----------------------------
# --- Get the GCC preinstalled image from Docker Hub --- 
FROM gcc:latest as build_layer

# --- Install neccessary packages ---
RUN apt-get update                                           \
    &&  apt-get install -y                                   \
          cmake

# --- Copy sources to container ---
COPY ./src /app/src
COPY ./CMakeLists.txt /app/CMakeLists.txt

WORKDIR /app/build

# --- Build ---
RUN cmake -DCMAKE_BUILD_TYPE=Release /app                    \
    &&  cmake --build .


ENTRYPOINT ["./HelloWorld"]