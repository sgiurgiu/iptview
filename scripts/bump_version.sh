#!/bin/bash

git --version
git remote remove origin
git remote add origin https://${GITLAB_USER_LOGIN}:${ACCESS_TOKEN}@${CI_SERVER_NAME}/${GITLAB_USER_LOGIN}/${CI_PROJECT_PATH}
cur_version=$(git describe --tags)
next_version=$(semver bump patch ${cur_version})
git tag -a ${next_version} -m "Release version ${next_version}"
git push origin --tags

