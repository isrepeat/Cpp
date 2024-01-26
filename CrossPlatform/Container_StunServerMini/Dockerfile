# You need pass to docker the root project context when build (to have access to 3rdParty):
# docker build -t <image_name> -f Dockerfile ..

# ----------------------------
#        Build layer          
# ----------------------------
# --- Get the GCC preinstalled image from Docker Hub --- 
FROM gcc:9.5.0 as build_layer

# --- Install neccessary packages ---
RUN apt-get update                                                                                   \
    &&  apt-get install -y                                                                           \
          libssl-dev                                                                                 \
          cmake                                                                                      \
          make                                                                                       \
          gcc                                                                                        \
          git

# NOTE: Docker context must be ../

# --- Copy Container_StunServerMini dir to container ---
COPY ./Container_StunServerMini  ./StunServerMini

WORKDIR ./StunServerMini
# --- Install 3rdParty dependencies ---
# ...

WORKDIR ./Release
RUN cmake .. && cmake --build .

# ----------------------------
#        Finish layer          
# ----------------------------
FROM gcc:9.5.0

# --- Install neccessary packages ---
COPY --from=build_layer  ./StunServerMini/Release/bin  ./StunServerMini/Release/bin

WORKDIR ./StunServerMini/Release/bin/
ENTRYPOINT ["./StunServerMini"]