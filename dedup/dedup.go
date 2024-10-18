package main

import (
	"crypto"
	_ "crypto/sha256"
	"encoding/hex"
	"flag"
	"fmt"
	"io"
	"io/fs"
	"maps"
	"os"
	"path/filepath"
	"slices"
	"sync"
)

var (
	root = flag.String("path", ".", "path from where to start walking")
	workers = flag.Int("j", 1, "number of workers to use")
	ignore_git_directory = flag.Bool("ignore-git", true, "ignore git directories")
)

type file struct {
	path string
	hash string
	size int64
}

// TODO: Deduplicate the output - if the whole directories are identical then we can mark whole directory as duplicate
func main() {
	flag.Parse()

	if *workers < 1 {
		*workers = 1
	}

	paths := make(chan string, 1000)

	go func() {
		filepath.WalkDir(*root, func(path string, d fs.DirEntry, err error) error {
			if *ignore_git_directory && filepath.Base(path) == ".git" {
				return filepath.SkipDir
			}
			if err != nil {
				fmt.Fprintln(os.Stderr, "failed to filepath.WalkDir:", err)
			} else if d.Type().IsRegular() {
				paths <- path
			}
			return nil
		})
		close(paths)
	}()

	wg := sync.WaitGroup{}

	files := make(chan file, 1000)

	for _ = range *workers {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for path := range paths {
				f, err := os.Open(path)
				if err != nil {
					fmt.Fprintf(os.Stderr, "failed to os.Open(%v): %s\n", path, err)
					continue
				}
				h := crypto.SHA256.New()
				n, err := io.Copy(h, f)
				f.Close()
				if err != nil {
					fmt.Fprintf(os.Stderr, "failed to io.Copy(%v): %s\n", path, err)
					continue
				}

				files <- file{path, hex.EncodeToString(h.Sum(nil)),n}
			}
		}()
	}

	go func() {
		wg.Wait()
		close(files)
	}()

	seen := map[string][]file{}

	for file := range files {
		seen[file.hash] = append(seen[file.hash], file)
	}

	seenSlice := slices.Collect(maps.Values(seen))

	slices.SortFunc(seenSlice, func(a, b []file) int {
		return len(b)*int(b[0].size) - len(a)*int(a[0].size)
	})

	for _, v := range seenSlice {
		if len(v) > 1 {
			fmt.Printf("%s size=%s, total=%s\n", v[0].hash, human(v[0].size), human(v[0].size * int64(len(v))))
			for _, file := range v {
				fmt.Println("  ", file.path)
			}
		}
	}
}

func human(n int64) string {
	x := float32(n)
	s := 0
	for x >= 1024 {
		x /= 1024
		s++
	}
	return fmt.Sprintf("%.2f%s", x, []string{"B", "KiB", "MiB", "GiB", "TiB"}[s])
}
