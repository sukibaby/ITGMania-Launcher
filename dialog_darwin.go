//go:build darwin

package main

import (
	"fmt"
	"os/exec"
)

func showUpdateDialog(latestVersion string) {
	text := fmt.Sprintf("A new version (%s) is available! Go to GitHub?", latestVersion)
	url := fmt.Sprintf("https://github.com/%s/%s/releases/latest", owner, repo)

	// AppleScript via osascript to show a dialog...
	cmd := exec.Command("osascript", "-e", fmt.Sprintf(`display dialog "%s" with title "Update Available"`, text))

	err := cmd.Run()
	if err == nil {
		exec.Command("open", url).Start()
	}
}
