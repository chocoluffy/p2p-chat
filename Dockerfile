FROM ubuntu:16.04

RUN apt-get update && apt-get install -y sudo && rm -rf /var/lib/apt/lists/* # to get sudo.

RUN sudo apt-get update # to update apt-get list.

RUN yes | sudo apt-get install qt4-qmake

RUN yes | sudo apt-get install libqt4-dev

RUN yes | sudo apt-get install --reinstall make

RUN yes | sudo apt-get install build-essential g++