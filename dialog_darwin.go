//go:build darwin

package main

import (
	"fmt"
	"os/exec"
)

func showUpdateDialog(latestVersion string) {
	text := fmt.Sprintf("A new version (%s) is available! Go to GitHub?", latestVersion)
	cmd := exec.Command("osascript", "-e", fmt.Sprintf(`display dialog "%s" buttons {"Maybe Later", "Go to GitHub"} default button 2 with title "Update Available"`, text))
	out, err := cmd.Output()
	if err == nil && string(out) != "" && string(out) != "button returned:Maybe Later" {
		exec.Command("open", "https://github.com/itgmania/itgmania/releases/latest").Start()
	}
}
