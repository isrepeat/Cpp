# ----------------------------
#          Building           
# ----------------------------
# --- Get the GCC preinstalled image from Docker Hub --- 
FROM gcc:latest as build_layer

# --- Install neccessary packages (Ubuntu) ---
RUN apt-get update                                                                                \
    &&  apt-get install -y                                                                        \
          libssl-dev                                                                              \
          cmake                                                                                   \
          make                                                                                    \
          gcc                                                                                     \
          git																		 

# --- Copy current dir to container and build ---
COPY . ./SimpleHttpServer
WORKDIR ./SimpleHttpServer

#EXPOSE 80/tcp
#EXPOSE 80/udp
#EXPOSE 8080/tcp
#EXPOSE 8080/udp

WORKDIR ./Release
RUN cmake .. && cmake --build .

ENTRYPOINT ["./SimpleHttpServer"]