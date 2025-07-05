// ITGmania Update Checker
//
// This program does the following things:
// 1. Checks for the latest release of ITGmania on GitHub.
// 2. If a newer version is available, it shows a dialog prompting the user to
//    visit the GitHub releases page.
// 3. Launches the game.
//
// The methods used to launch the game and show the dialog are platform-specific.
//
// NOTE: Be sure to compile the Windows binary with the `-ldflags "-H windowsgui"`
// flag to avoid a console window from appearing.

package main

import (
	"encoding/json"
	"fmt"
	"net/http"
)

const (
	version = "1.1.0"
	owner   = "itgmania"
	repo    = "itgmania"
)

type GitHubRelease struct {
	TagName string `json:"tag_name"`
}

func CheckForLatestGitHubRelease() (string, error) {
	url := fmt.Sprintf("https://api.github.com/repos/%s/%s/releases/latest", owner, repo)
	resp, err := http.Get(url)
	if err != nil {
		return "", fmt.Errorf("ITGmania updater failed to fetch latest release: %v", err)
	}

	// Close the HTTP response body
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("ITGmania updater encountered an unexpected status code when checking for updates: %d", resp.StatusCode)
	}

	var release GitHubRelease
	if err := json.NewDecoder(resp.Body).Decode(&release); err != nil {
		return "", fmt.Errorf("ITGmania updater failed to parse response: %v", err)
	}

	return release.TagName, nil
}

func main() {
	latestVersion, err := CheckForLatestGitHubRelease()
	if err != nil {
		launchTarget()
		return
	}
	showUpdateDialog(latestVersion)
}
