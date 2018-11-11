```bash
# the actual working code.
docker run -it -e DISPLAY=$IP:0 -v /tmp/.X11-unix:/tmp/.X11-unix -v $PWD/lab3:/app ubuntu-test
```
