//go:build linux

package main

import (
	"os/exec"
)

const targetExe = "itgmania"

func launchTarget() {
	cmd := exec.Command(targetExe)
	cmd.Start()
}
