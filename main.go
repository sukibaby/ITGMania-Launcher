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
		return "", fmt.Errorf("failed to fetch latest release: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("unexpected status code: %d", resp.StatusCode)
	}

	var release GitHubRelease
	if err := json.NewDecoder(resp.Body).Decode(&release); err != nil {
		return "", fmt.Errorf("failed to parse response: %v", err)
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
