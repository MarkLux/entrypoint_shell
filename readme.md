This program is using for pod container launch wait

build:

```
go build -o entrypoint_shell
```

usage:

```
entrypoint_shell \
-waitpath /home/waitfile \
-file 0 \
-entrypoint "tail -f /dev/null"
```

will wait until `/home/waitfile/0` was create and then exec `tail -f /dev/null` (using same process, will not create new process.)