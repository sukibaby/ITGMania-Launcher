//go:build darwin

package main

import (
	"os/exec"
)

const targetExe = "ITGmania.app"

func launchTarget() {
	cmd := exec.Command(targetExe)
	cmd.Start()
}
