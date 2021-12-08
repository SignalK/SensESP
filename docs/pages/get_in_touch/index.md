---
layout: default
title: Get in touch
nav_order: 45
---

# Get in touch

- Slack channel

# Contributing Code to SensESP

If you like SensESP and want to make it even better, please do! It's written in C++ and uses some JSON. There are always some active Issues that you could tackle here on the GitHub page, or feel free to write the interface for a new Sensor, or create a new Transport - there are lots of great examples of these two things. But if you're going to contribute, follow these guidelines:

- New contributions should follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) whenever reasonable. SensESP doesn't have anything to do with Google but they just provide a well-written and reasonable style guide, and rather than start arguing about every style detail, let's follow a guide written by some smart people. You don't have to read the whole guide (it's long and goes to rather obscure details) but things like naming and formatting should be followed whenever possible. The first revisions of SensESP were written following this style guide. Since then, SensESP hasn't stayed quite consistent, even on the API level. To quickly reformat existing source files in Visual Studio Code according to the preferred style, press Shift-Alt-F to apply the "Reformat Document" command.

- Provide descriptive commit messages. For a great introduction, read this: https://chris.beams.io/posts/git-commit/.

- Try to keep your git commits small and atomic, that is, one commit should only introduce a single logical change. That change may affect multiple files, of course, but if you need to use the "and" word in the commit message, your commit might be doing too many things. Also, your commits should tell a story: if you change file `foo.cpp` and you find out you need to fix something to get that change to work, instead of having two commits "Implemented foblinator" and "Adjusted foblinator parameters" you should squash them together. In other words, hide your mistakes!

- In an ideal world, no commit should break the build. That is, if you create commits 1, 2, 3, and 4, you should be able to checkout any of these individual commits and build and upload the project successfully. This is not always easy, so this is more of an aspirational goal, but if you _can_ achieve this, debugging issues may become a _lot_ easier (`git bisect` is your friend!).

- If you reformat existing code, do that always in separate commits. It's very difficult to review PRs that mix functional changes and code reformatting in the same commits.

- For your own benefit, it is recommended that you create a new Git branch for each PR. It doesn't necessarily change the end result, but it's very likely that you'll be asked to make some changes in the PR, and if you're working out of your master branch, you might end up in a world of pain. Hence, make a unique branch for each PR. If you don't already have a Git process that works perfectly, and that includes making a new branch for each PR, then please follow the steps below exactly.

- Do not commit PRs with internal merge commits. The information that you've merged changes from upstream at some point benefit no-one, probably not even you. You can avoid those merge commits by doing a `git pull --rebase upstream master` (or `git fetch` followed by `git rebase`, but that's more intricate) during your own work.

- Normally, you should not need to modify `platformio.ini` for your PR. However, if you're adding code that requires a new library dependency, add the dependency both to `library.json` and `platformio.ini`. If you have questions, the best place to discuss SensESP is on [Slack](https://signalk-dev.slack.com), in the `#sensors` channel.

## Write Comments That Create Interactive Documentation

SensESP is set up to generate Doxygen documentation files from the comments in the .h files, if you follow the appropriate formatting for your comments. So if you create a new .h file, or if you're working on an existing one, please use the following comment format for every Class definition.

```c++
  /**
   * @brief One-line description of what this class does
   *
   * More detailed description of what the class does, along with any
   * specific information an end-user might need to know to use the
   * class.
   *
   * @param pin The GPIO pin to read. On ESP8266, always A0. On ESP32, at
   * the moment only ADC channel 1 (pins 32..39) is supported.
   * @param read_delay Time delay between consecutive readings, in ms
   * @param config_path Configuration path for the sensor
   * @param output_scale The maximum scale of the output
   */
class YourClassName : public NumericSensor {
 public:
  YourClassName(uint8_t pin = A0, uint read_delay = 200, String config_path = "",
              float output_scale = 1024.);
  (the rest of the Class definition goes here)
```

In the above example, `@brief` identifies the brief description; `@param pin` starts the description of a parameter to the constructor called `pin`.

If your Class uses a template, you can describe the template parameters like this: `@tparam C The class of the Consumer for this Transform`.

All other Doxygen commenting key words are recognized, so use them if you like.

## GitHub / Git / SensESP Overview (skip if you already know how Git / GitHub work)

- Your Git SensESP "working directory" (the directory where Git will store all of its info, manage commits, etc.) is the directory where you clone your fork of SensESP into. For example: ~/Documents/GitHubProjects/SensESP

- You will have two Git remotes:
  `origin` is your GitHub page for your fork of SensESP: https://github.com/YOUR_GITHUB_NAME/SensESP.git.
  `upstream` is the main SensESP page: https://github.com/SignalK/SensESP.git.

- You do all of your updating of files (code changes you make) to your "development directory" on your local computer, then copy them to your Git "working directory". From there, the changes become "commits", and are sent up to "origin". Because "origin" is a fork of the main SensESP repo (called "upstream"), you then use GitHub to make a Pull Request from "origin" to "upstream" (from your GitHub repo of the SensESP fork, to the main SensESP repo).

- By always submitting your code changes in a new Git `branch`, your commits can't be mixed up with other commits (which causes big problems, which is why we strongly recommend all PRs to be in a separate branch).

## How to work on SensESP itself

Unless you're an experienced git pro, please read [this article](https://akrabat.com/the-beginners-guide-to-contributing-to-a-github-project/) about how to start contributing to a GitHub project before you do anything else. It's a short read, and contains an excellent description of many of the steps below.

After you've read the article, complete steps 1 and 2 in the article, to create your fork of SensESP, and your local clone of that fork on your computer.

Normally, any new project using SensESP should create a separate PlatformIO project and only include SensESP as a dependency. However, if you're going to work on SensESP itself, that approach becomes cumbersome. So, break the rules and create a `main.cpp` for your own development purposes. It should define an environment for your own testing purposes. Do _not_ commit that file to the git repo, however. It's yours, and yours only, although it might be a good idea to provide it as an example for the new stuff you've been working on. Make sure your `main.cpp` works and you can build and upload the project and it works. Now, you can start hacking away! Create a branch, modify code, upload, test, create commits, fix code, fix commits, etc.

## Creating a Pull Request for SensESP

If you are new to Git and GitHub, GitHub has some wonderful guides on how git works and how to contribute to existing projects. To contribute to a project, you normally need to _fork_ it and then do modifications in that fork. This guide walks you through the basic process: https://guides.github.com/activities/forking/

If you need a more thorough explanation on Git and the git workflow, see this page: https://guides.github.com/introduction/git-handbook/

At this point, you should try creating a fork and cloning it locally. Or, if you already have cloned SensESP and done some modifications, create a fork and add it as a new remote with `git remote`.

When working on the modifications, it is highly recommended to commit often. Creating a huge amount of changes and then trying to create good commits rarely works out well.

The SensESP project targets both ESP8266 and ESP32. Even if you wouldn't have both hardware available, at least try to ensure you can build the project against both targets. Usually that already goes a long way.

Once you think your work is shiny and perfect, you want to make a pull request. That's a two-stage process: you first need to push the changes to your own fork (that is, publish them somewhere), and then request that your changes are pulled into SensESP from your fork (hence, a "Pull Request").

If your development work has taken some time, it is quite likely that some unrelated PRs have been merged to the main repository meanwhile. To make sure your work can still be applied, you need to pull those changes back to your own repo and then rebase your local branch onto the updated master.

Some more detailed instructions follow.

1. Get PlatformIO ready to start development work (see above).

3. Open terminal and go to the Git SensESP working directory (for example, ~/Documents/GitHubProjects/SensESP).

4. `git checkout master`

5. `git pull --rebase upstream master` will pull down all changes from the main SensESP repo on GitHub and add them on your local Git repository. If you have made local changes in `master` (which you shouldn't have!), the upstream commits are inserted below your own ones. If same files have been modified locally and in the upstream, you may get a conflict. In that case google for "resolve git merge conflicts".

6. `git checkout -b <newBranchName>` creates a new Git branch for all the changes in this PR, and checks it out.

9. `git status` will show what files have changed in the working directory for newBranchName (but they're just there - not committed yet).

10. `git add <changed_file>` will "stage" one changed file. Do NOT attempt to stage or commit all changed or new files because that WILL result in your commits containing files that shouldn't be there. Do this (`git add <changed_file>`) for each of the files you want to include in this commit. For example, if you are creating a new Transform, you will probably have two new files: `newTransform.h` and `newTransform.cpp`. Do `git add` for each of them, so that both of them, but ONLY them, will be included in this commit.

11. `git status` again shows all the files in the staging area (ready to be committed) for newBranchName.

12. `git commit -m "type your commit message here"` creates the commit for all staged files in NewBranchName, and adds your commit message to it. For more involved commits, it is better to just do `git commit` and write a more detailed multiline commit message in the editor window that opens.

    At this point, a commit has been created, but only locally. It's not on GitHub yet.

    IF YOU FORGOT A FILE: you can amend a commit before you push it to "origin" with the "--amend" modifier - see below.

    IF YOU NEED TO CLEAN YOUR COMMITS: an interactive rebase is your friend: `git rebase -i master`.

13. `git push --set-upstream origin <newBranchName>` pushes the commit to "origin", which is your SensESP fork page on GitHub.

14. Go to YOUR_GITHUB_NAME/SensESP on GitHub and refresh the page. There will be a section near the top prompting you to create a new Pull Request. Start the process, review all the changes in all the files to make sure they're what you want (and ONLY what you want), then create the PR.

## Update an Existing Pull Request Before it is Merged
To Update a PR with changes before merging the PR:
Do these steps in the above section:
2.
3.
`git checkout branchName` - for the branch you created for this PR
`git status` to see the current status of branchName - should say `nothing to commit, working directory clean`
8.
9.
10.
12.
13.
14. `git rebase -i` to embed the requested changes to your earlier commits.

Go to Signalk/SensESP (the main repo) on GitHub and bring up the PR. At the bottom of all the activity, you should see your most recent commit message. If you don't, something didn't work right.

## After Your PR Has Been Merged to the Main SensESP Repo
After the PR has been approved and merged, you can delete the branch, with the [Delete branch] button near the bottom of the PR (it appears after the PR has been merged). That will delete it from your own fork. Then do "git branch -d branchName" to delete it locally. These steps are optional; you might want to prefer having them around, for some reason.

THEN, do all the numbered steps in "Get PlatformIO Ready to Start Development", below.

## Using your modified SensESP repo locally

If you have created `my_cool_project` and want it to use your local modified version of SensESP, do the following:

In the platformio.ini file of the SensESP Project, add this in the `lib_deps` section:
   `file:///home/butch/Documents/GitHubProjects/SensESP` (obviously this will be different on your computer - be sure to start with THREE forward slashes, not two)

Comment out the existing line in that section that points to the SensESP GitHub repo:
`;    https://github.com/SignalK/SensESP.git`

That tells PlatformIO to pull all the files for the SensESP library from your Git "working directory" on your computer, rather than from the latest master on SignalK/SensESP. It pulls files from there whenever there is no `.pio` directory in the Project. Then PlatformIO uses (compiles from) the ones that it has placed under the newly-created `.pio` directory.


## Miscellaneous Helpful Git Info
Excellent article on how to updo things in git: (https://github.blog/2015-06-08-how-to-undo-almost-anything-with-git/)

[Git Cheat Sheet](https://rubygarage.org/blog/most-basic-git-commands-with-examples)

-`git remote -v` shows the remotes that are set up - should be only "origin" (your online repo fork) and "upstream" (the main online repo).

-`git status` shows what files will be included in the next commit, for whatever branch you currently have checked out

-`git branch` shows all branches, and uses an asterisk to show you which one is currently checked out

-`git checkout <branchName>` (makes branchName the current working branch

-If you forgot to include a particular file in the commit, and you haven't yet pushed the commit:
`git add file-i-forgot-to-add.html`

-`git commit --amend -m "Add the remaining file"`

-`git log --oneline` shows you the history of commits, including the ID of each commit. Needed if you need to revert a commit. Type the letter `q to exit the log.

-`git revert <commitID> -m "Revert <msg of the commit you're reverting>"`   will revert the commit with that commitID, and give it that revert msg.
