package main

import (
	"flag"
	"fmt"
	"math"
	"os"
	"path"
	"slices"
	"sync"
)

var skiplist []string = []string {
	".git",
	"target",
}

var (
	root = flag.String("root", ".", "from where search should start")
	maxLevel = flag.Int("level", -1, "how deep program should search")
)


func main() {
	flag.Parse()
	if *maxLevel <= 0 {
		*maxLevel = math.MaxInt
	}

	empty := make(chan string, 10)
	errors := make(chan error, 10)

	wg := sync.WaitGroup{}

	var visit func(base string, level int) = nil
	visit = func(base string, level int) {
		defer wg.Done()

		entries, err := os.ReadDir(base)
		if err != nil {
			errors <- fmt.Errorf("failed to os.ReadDir(%s): %s", base, err)
			return
		}

		if len(entries) == 0 {
			empty <- base
			return
		}

		if level == *maxLevel {
			return
		}

		for _, entry := range entries {
			if slices.Contains(skiplist, entry.Name()) {
				continue
			}

			p := path.Join(base, entry.Name())
			if isSymlink(p) {
				continue
			}

			if entry.Type().IsDir() {
				wg.Add(1)
				go visit(p, level + 1)
			}
		}
	}

	wg.Add(1)
	go visit(*root, 0)

	// This closed channel is a weird solution
	closed := make(chan struct{})

	go func() {
		wg.Wait()
		closed <- struct{}{}
	}()

	for {
		select {
		case <-closed:
			// Since select is nondeterministic we must exit it first,
			// then we can close the channels. Go is weird
			close(empty)
			close(errors)
			return
		case dir := <-empty:
			fmt.Println(dir)
		case err := <-errors:
			fmt.Fprintln(os.Stderr, "[ERROR] ", err)
		}
	}
}

func isSymlink(path string) bool {
	info, err := os.Lstat(path)
	return err != nil || info.Mode()&os.ModeSymlink != 0
}
