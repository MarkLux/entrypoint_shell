package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"strings"
	"syscall"

	"path/filepath"

	"github.com/fsnotify/fsnotify"
)

func main() {

	// handle with cli args
	waitPathArg := flag.String("waitpath", "", "the path to wait file create")
	fileNameArg := flag.String("file", "", "the file name to wait for create")
	realEntrypoint := flag.String("entrypoint", "", "the real entrypoint to exec")

	flag.Parse()

	if *waitPathArg == "" || *fileNameArg == "" || *realEntrypoint == "" {
		panic(fmt.Sprintf("invalid args: waitpath: %s, file: %s, entrypoint: %s", *waitPathArg, *fileNameArg, *realEntrypoint))
	}

	// parse the entrypoint
	bin, args := parseEntrypoint(*realEntrypoint)

	binary, err := exec.LookPath(bin)
	if err != nil {
		panic(err)
	}

	// wait for file create
	err = waitForFileCreate(*waitPathArg, *fileNameArg)

	if err != nil {
		panic(err)
	}

	env := os.Environ()
	if err := syscall.Exec(binary, args, env); err != nil {
		// if exec sys call succeed, the following code will not be executed.
		panic(err)
	}
}

// return binary and args
func parseEntrypoint(entrypoint string) (bin string, args []string) {
	args = strings.Split(entrypoint, " ")
	bin = args[0]
	return
}

// wait until for the file create.
func waitForFileCreate(path string, file string) error {
	watcher, err := fsnotify.NewWatcher()
	if err != nil {
		return err
	}
	defer watcher.Close()

	err = watcher.Add(path)
	if err != nil {
		return err
	}

	for {
		select {
		case event, ok := <-watcher.Events:
			if !ok {
				return fmt.Errorf("watcher event error for path: %s", path)
			}
			if event.Op&fsnotify.Create != 0 {
				if event.Name == filepath.Join(path, file) {
					return nil
				}
			}
		case err, ok := <-watcher.Errors:
			if !ok {
				return err
			}
		}
	}
}
