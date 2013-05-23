# Effects processing on ffmpeg yuv4mpegpipe

```shell
$ ./setup.sh && ./configure && make test
```

Requires: https://github.com/AndyA/jsondata

```shell
$ ffmpeg -i infile.mov -f yuv4mpegpipe - | \
    timewarp config.json | \
    ffmpeg -y -f yuv4mpegpipe -i - -c:v libx264 -b:v 3000k outfile.ts
```

Andy Armstrong, andy@hexten.net
