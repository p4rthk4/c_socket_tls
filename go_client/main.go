package main

import (
	"fmt"
	"sync"
)

func main() {

	fileList := getFileList();

	numRoutines := 5

	// Distribute files among routines
	fileChunks := distributeFiles(fileList, numRoutines)

	// Wait group for synchronization
	var wg sync.WaitGroup

	// Launch Go routines
	for i, files := range fileChunks {
		wg.Add(1)
		go processFiles(files, &wg, i)
		fmt.Printf("Go routine %d started\n", i+1)
	}

	wg.Wait()

}


func distributeFiles(fileNames []string, numRoutines int) [][]string {
	chunkSize := (len(fileNames) + numRoutines - 1) / numRoutines // Round up
	chunks := make([][]string, numRoutines)

	for i := 0; i < numRoutines; i++ {
		start := i * chunkSize
		end := start + chunkSize
		if end > len(fileNames) {
			end = len(fileNames)
		}
		chunks[i] = fileNames[start:end]
	}

	return chunks
}

func processFiles(files []string, wg *sync.WaitGroup, r int) {
	defer wg.Done()
	for _, file := range files {
		client(file)
	}
}
