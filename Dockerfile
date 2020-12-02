FROM ubuntu:latest

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive \
    apt-get install -yy --no-install-recommends \
        g++-10 make cmake curl git ca-certificates ssh
RUN apt-get clean -y
RUN ln -s g++-10 /usr/bin/g++
RUN ln -s gcc-10 /usr/bin/gcc
