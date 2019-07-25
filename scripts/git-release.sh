#!/usr/bin/env bash

# Create a new release on the master branch, "moving" the latest commit on
# the develop branch to the master branch. The new commit on the master branch
# is tagged with 'version_string'. Both the new commit on the master branch
# and the new tag are then pushed to 'origin'.

set -euo pipefail

git rev-parse --git-dir 1>/dev/null # exit if not inside Git repo

if [[ "${#}" -ne 1 ]]; then
    echo "Usage: ${0} version_string"; exit 1
fi

echo "== Checking out 'master'"
git checkout master
echo "== Merging latest commit from 'develop' into 'master'"
git merge develop --squash --strategy=recursive --strategy-option=theirs
echo "== Creating new commit on 'master' with commit message '${1}'"
git commit --message "${1}"
echo "== Tagging the just created commit on 'master' with '${1}'"
git tag "${1}"
echo "== If everything looks correct please manually push everything: "
echo "git push origin master && git push origin %s" "${1}"
