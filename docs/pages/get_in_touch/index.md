---
layout: default
title: Get in touch
nav_order: 45
---

# Get In Touch

By far the best way to get in touch with someone who can help you with a SensESP issue is through the #sensors channel on the Signalk-dev Slack. If you're not already a member there (it's free), [start here](http://slack-invite.signalk.org/).

Please do NOT create an Issue on the SensESP GitHub repo unless you have already posted about it on the #sensors channel on Slack. You might think it's a bug, but you're just doing something wrong. You might want to request an enhancement, but it may already do what you want. Ask first, then create an Issue only if it's still appropriate.

## Contributing Code to SensESP

If you like SensESP and want to make it even better, please do! It's written in C++ and uses some JSON. There are always some active Issues that you could tackle here on the GitHub page, or feel free to write the interface for a new Sensor, or create a new Transform - there are lots of great examples of these two things to get you started. But if you're going to contribute, please follow these guidelines:

- New contributions should follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) whenever reasonable. SensESP doesn't have anything to do with Google but they provide a well-written and reasonable style guide, and rather than start arguing about every style detail, let's follow a guide written by some smart people. You don't have to read the whole guide (it's long and goes to rather obscure details) but things like naming and formatting should be followed whenever possible. The first revisions of SensESP were written following this style guide. Since then, SensESP hasn't stayed quite consistent, even on the API level. To quickly reformat existing source files in Visual Studio Code according to the preferred style, press Shift-Alt-F to apply the "Reformat Document" command.

- Provide descriptive commit messages. For a great introduction, read this: https://chris.beams.io/posts/git-commit/.

- Try to keep your git commits small and atomic. That is, one commit should introduce only a single logical change. That change may affect multiple files, of course, but if you need to use the word "and" in the commit message, your commit might be doing too many things. Also, your commits should tell a story: if you change file `foo.cpp` and you find out you need to fix something to get that change to work, instead of having two commits "Implemented foblinator" and "Adjusted foblinator parameters" you should squash them together. In other words, hide your mistakes!

- In an ideal world, no commit should break the build. That is, if you create commits 1, 2, 3, and 4, you should be able to checkout any of these individual commits and build and upload the project successfully. This is not always easy, so this is more of an aspirational goal, but if you _can_ achieve this, debugging issues may become a _lot_ easier (`git bisect` is your friend!).

- If you reformat existing code, always do that in separate commits. It's very difficult to review PRs that mix functional changes and code reformatting in the same commits.

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

## Using Your Modified SensESP Files

If you create `my_cool_project` and want it to use your locally modified version of SensESP, do the following:

In the platformio.ini file of your Project, add this in the `lib_deps` section:
   `file:///home/butch/Documents/GitHubProjects/SensESP` (Obviously this will be different on your computer - be sure to start the path with THREE forward slashes, not two.)

Then comment out the existing line in that section that points to the SensESP GitHub repo:
`;    https://github.com/SignalK/SensESP.git`

That tells PlatformIO to pull all the files for the SensESP library from your git "working directory" on your computer, rather than from the latest version on SignalK/SensESP. Whenever there is no `.pio` directory in the Project's directory, PlatformIO will pull the files from wherever `lib_deps` points, then compile those. So any time you change where `lib_deps` points to, you need to exit PlatformIO, delete the `.pio` directory in your Project directory, then restart PlatformIO and give it a minute to update everthing.
