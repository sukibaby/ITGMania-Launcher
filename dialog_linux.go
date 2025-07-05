//go:build linux

package main

import (
	"fmt"
	"os/exec"
)

func showUpdateDialog(latestVersion string) {
	text := fmt.Sprintf("A new version (%s) is available! Go to GitHub?", latestVersion)
	cmd := exec.Command("zenity", "--question", "--title=Update Available", "--text="+text)
	err := cmd.Run()
	if err == nil && cmd.ProcessState.Success() {
		exec.Command("xdg-open", "https://github.com/itgmania/itgmania/releases/latest").Start()
	}
}
