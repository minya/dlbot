
# Сборка ---------------------------------------

FROM gcc:latest as build


# Установим рабочую директорию для сборки boost
WORKDIR /cmake
RUN wget https://github.com/Kitware/CMake/releases/download/v3.15.2/cmake-3.15.2.tar.gz
RUN tar -zxvf cmake-3.15.2.tar.gz
RUN cd cmake-3.15.2 && ./bootstrap && make && make install

WORKDIR /boost

RUN apt-get update 
RUN apt-get install -y libssl-dev libcurl4-openssl-dev git zlib1g-dev
RUN wget -O boost_1_76_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.76.0/boost_1_76_0.tar.gz/download 
RUN tar xzvf boost_1_76_0.tar.gz
WORKDIR /boost/boost_1_76_0
RUN ./bootstrap.sh --prefix=/usr/
RUN ./b2
RUN ./b2 install
