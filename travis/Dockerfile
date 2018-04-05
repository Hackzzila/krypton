FROM ubuntu:xenial

RUN dpkg --add-architecture i386
RUN apt-get update
RUN apt-get install -y curl

RUN curl -sL https://deb.nodesource.com/setup_8.x | bash -
RUN apt-get install -y nodejs make gcc g++ gcc-5-multilib g++-5-multilib \
  gcc-5-aarch64-linux-gnu  g++-5-aarch64-linux-gnu \
  gcc-5-multilib-arm-linux-gnueabihf g++-5-multilib-arm-linux-gnueabihf \
  gcc-aarch64-linux-gnu g++-aarch64-linux-gnu gcc-arm-linux-gnueabihf \
  g++-arm-linux-gnueabihf \
  linux-libc-dev:i386

RUN mkdir -p /usr/src
WORKDIR /usr/src

CMD ["node", "--throw-deprecation", "build"]
