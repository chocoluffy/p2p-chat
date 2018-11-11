# Notes

- invoke functions under the correct scope! for example, put `writeDatagram()` functions under the correct NetSocket environment.

# Testing

### into docker to build.
	 * at `./lab3` current folder:  `docker run -it -v $PWD:/app ubuntu-test`
	 * `qmake-qt4`
	* 	`make clean && make`

### outside docker to debug and test.

# Configuration

```bash
# the actual working code.
docker run -it -e DISPLAY=$IP:0 -v /tmp/.X11-unix:/tmp/.X11-unix -v $PWD/lab3:/app ubuntu-test
```

---

```bash
docker build . -t ubuntu-test

# sync current folder and X11 config.
docker run -it -v $PWD/lab3:/app -v /tmp/.X11-unix:/tmp/.X11-unix:ro -e DISPLAY=$IP:0 ubuntu-test
```


```bash
# dockerfile.
docker run --name=ubuntu -it -e DISPLAY=$IP:0 -v $PWD/lab3:/app -p 5000:80 ubuntu:16.04

apt-get update && apt-get install -y sudo && rm -rf /var/lib/apt/lists/* # to get sudo.

sudo apt-get update # to update apt-get list.

sudo apt-get install qt4-qmake

sudo apt-get install libqt4-dev

sudo apt-get install --reinstall make

sudo apt-get install build-essential g++
```