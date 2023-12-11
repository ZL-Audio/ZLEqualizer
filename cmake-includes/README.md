# CMake Includes for Pamplejuce

Hi there!

## What is this?

It's most of the actual CMake functionality used by [Pamplejuce](https://github.com/sudara/pamplejuce), my template repository for plugins in the JUCE framework.

## Why is this its own git submodule?

It's to help projects built by the template pull in the lastest changes.

[Pamplejuce](https://github.com/sudara/pamplejuce) is a template repository. Unlike most "dependencies," when you hit "Create Template" you are literally copying and pasting the code. Which sorta sucks, as people can't get fixes or updates.

## Why would I want updates?

For at least the gritty CMake details, there are fixes, improvements and additional functionality being added. 

In the best case, as a submodule, you can pull in the fixes and improvements.

In the worst case, this seperate repo will help you see what exactly changed in Pamplejuce.

## Is it risky?

It could be! 

As of 2023, Pamplejuce is still being changed around a bunch, with the goal of being a better and better ecosystem for developers.

That means there could be breakage when you pull. 

## What changed recently tho?

See [CHANGELOG.md](CHANGELOG.md).
