//go:build linux

package main

import (
	"fmt"
	"os/exec"
)

func showUpdateDialog(latestVersion string) {
	text := fmt.Sprintf("A new version (%s) is available! Go to GitHub?", latestVersion)
	url := fmt.Sprintf("https://github.com/%s/%s/releases/latest", owner, repo)
	cmd := exec.Command("dialog", "--yesno", text, "10", "50") // I doubt this is the best way to do this. Is it?
	err := cmd.Run()
	if err == nil && cmd.ProcessState.Success() {
		exec.Command("xdg-open", url).Start()
	}
}
