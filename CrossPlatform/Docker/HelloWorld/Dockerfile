# --- Get the GCC preinstalled image from Docker Hub --- 
FROM gcc:latest as build_layer

ARG BUILDOS
ARG BUILDARCH
ARG BUILDVARIANT
ARG BUILDPLATFORM
ARG TARGETPLATFORM

ARG HOST_DISTRIBUTION=centos
RUN echo "BuildPlatform = $BUILDPLATFORM (${BUILDOS} .. ${BUILDARCH} .. ${BUILDVARIANT})"
RUN echo "HOST_DISTRIBUTION = ${HOST_DISTRIBUTION}"

COPY . ./HelloWorld
WORKDIR ./HelloWorld


RUN if [ "HOST_DISTRIBUTION" = "ubuntu" ]; then \
      sed -i 's/\r//' /scriptUbuntu.sh && \
      chmod +x /scriptUbuntu.sh; \
    elif [ "HOST_DISTRIBUTION" = "centos" ]; then \
      sed -i 's/\r//' /scriptCentOs.sh && \
      chmod +x /scriptCentOs.sh; \
    fi

#RUN if [ "HOST_DISTRIBUTION" = "ubuntu" ]; then ./scriptUbuntu.sh; elif [ "HOST_DISTRIBUTION" = "centos" ]; then ./scriptCentOs.sh; fi

RUN if [ "HOST_DISTRIBUTION" = "ubuntu" ]; then \
      echo "Ubuntu commands ..."; \
    elif [ "HOST_DISTRIBUTION" = "centos" ]; then \
      echo "CentOs commands ..."; \
    else \
      echo "Distribution = ${HOST_DISTRIBUTION} not supported"; \
    fi