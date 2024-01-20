#!/usr/bin/env python3

'''
This script fetches the release notes from GitHub for the NightlyRelease, and
sorts release notes into "Notable Commits" (for commits that touched
core-changes.txt) and "Additional Commits" (for those that did not).
'''

import json
import os
import re
import subprocess
import sys
import urllib.request

COMMIT_URL_REGEX = re.compile('https://github.com/polserver/polserver/commit/([0-9a-f]{40})')
NIGHTLY_RELEASE_URL = 'https://api.github.com/repos/polserver/polserver/releases/tags/NightlyRelease'

class GitBlame:
      def __init__(self, path):
            process = subprocess.Popen(['git', 'blame', '-l', '--', path], stdout=subprocess.PIPE, text=True)

            self.commits = set()

            for line in process.stdout:
                commit = line[0:line.index(' ')]
                self.commits.add(commit)

            process.wait()

class Main:
    def run(self):

        mydir = os.path.dirname(os.path.realpath(__file__))

        try:
            changes = GitBlame(os.path.join(mydir,'..','pol-core','doc','core-changes.txt'))

            request = urllib.request.Request(NIGHTLY_RELEASE_URL)
            request.add_header('Accept', 'application/vnd.github+json')

            notable_commits = []
            additional_commits = []

            with urllib.request.urlopen(request) as response:
                url_content = response.read().decode('utf-8')
                json_data = json.loads(url_content)
                body = json_data['body']

                for line in body.splitlines():
                    matches = COMMIT_URL_REGEX.search(line)
                    if matches:
                        commit_id = matches[1]
                        if commit_id in changes.commits:
                            notable_commits.append(line)
                        else:
                            additional_commits.append(line)

            print('## Notable Commits')
            for line in notable_commits:
                print(line)
            print('## Additional Commits')
            for line in additional_commits:
                print(line)

        except subprocess.CalledProcessError as e:
            print(f"Error: Command '{e.cmd}' failed with return code {e.returncode}.")
        except urllib.error.URLError as e:
            print(f'Error: Unable to fetch URL. {e}')
            sys.exit(1)
        except Exception as e:
            print(f'Error: {e}')
            sys.exit(1)


if __name__ == '__main__':
    Main().run()
