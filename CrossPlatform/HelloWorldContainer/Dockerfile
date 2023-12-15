# ----------------------------
#          Building           
# ----------------------------
FROM gcc:latest as build

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

# ----------------------------
#          Launch           
# ----------------------------
FROM ubuntu:latest

WORKDIR /app

# -- Copy the target build to current work directory (from previous "build" container) ---
COPY --from=build /app/build/HelloWorld .

ENTRYPOINT ["./HelloWorld"]