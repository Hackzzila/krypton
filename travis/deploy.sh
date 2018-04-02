#!/bin/bash
# Adapted from https://gist.github.com/domenic/ec8b0fc8ab45f39403dd.
set -e

# For revert branches, do nothing
if [[ "$TRAVIS_BRANCH" == revert-* ]]; then
  echo -e "\e[36m\e[1mBuild triggered for reversion branch \"${TRAVIS_BRANCH}\" - doing nothing."
  exit 0
fi

# For docs builds, do nothing
if [[ "$TRAVIS_COMMIT_MESSAGE" ==  "Docs build for"* ]]; then
  echo -e "\e[36m\e[1mBuild triggered for docs build commit - doing nothing."
  exit 0
fi

DONT_COMMIT=false

if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
  echo -e "\e[36m\e[1mBuild triggered for PR #${TRAVIS_PULL_REQUEST} to branch \"${TRAVIS_BRANCH}\" - not commiting"
  SOURCE_TYPE="pr"
  DONT_COMMIT=true
elif [ -n "$TRAVIS_TAG" ]; then
  echo -e "\e[36m\e[1mBuild triggered for tag \"${TRAVIS_TAG}\"."
  SOURCE=$TRAVIS_TAG
  SOURCE_TYPE="tag"
else
  echo -e "\e[36m\e[1mBuild triggered for branch \"${TRAVIS_BRANCH}\"."
  SOURCE=$TRAVIS_BRANCH
  SOURCE_TYPE="branch"
fi

# Checkout branch
git checkout $TRAVIS_BRANCH

# Run the build
npm run docs

if [ $DONT_COMMIT == true ]; then
  echo -e "\e[36m\e[1mNot commiting - exiting early"
  exit 0
fi

# Initialise some useful variables
REPO=`git config remote.origin.url`
SSH_REPO=${REPO/https:\/\/github.com\//git@github.com:}
SHA=`git rev-parse --verify HEAD`

# Decrypt and add the ssh key
ENCRYPTED_KEY_VAR="encrypted_${ENCRYPTION_LABEL}_key"
ENCRYPTED_IV_VAR="encrypted_${ENCRYPTION_LABEL}_iv"
ENCRYPTED_KEY=${!ENCRYPTED_KEY_VAR}
ENCRYPTED_IV=${!ENCRYPTED_IV_VAR}
openssl aes-256-cbc -K $ENCRYPTED_KEY -iv $ENCRYPTED_IV -in travis/deploy-key.enc -out deploy-key -d
chmod 600 deploy-key
eval `ssh-agent -s`
ssh-add deploy-key

# Commit and push
git add -f docs
git config user.name "Travis CI"
git config user.email "$COMMIT_AUTHOR_EMAIL"
git commit -m "Docs build for ${SOURCE_TYPE} ${SOURCE}: ${SHA}" || true
git push $SSH_REPO $TARGET_BRANCH