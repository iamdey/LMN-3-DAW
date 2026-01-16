# Release procedure

_(This document is meant for maintainers of this repository.)_

- Make sure it's all green.
- The ./CHANGELOG.md file is filled

Choose the correct semantic version according to the changes.
Set manually the version and the date of the release in the CHANGELOG file. e.g.

```diff
+ ## 0.4.3 - 1970/1/1
```

and at the bottom of the file

```diff
 [unreleased]:
+  https://github.com/FundamentalFrequency/LMN-3-DAW/compare/v0.4.3...HEAD
-  https://github.com/FundamentalFrequency/LMN-3-DAW/compare/v0.4.2...HEAD
+[0.4.3]:
+  https://github.com/FundamentalFrequency/LMN-3-DAW/compare/v0.4.2...v0.4.3
 [0.4.2]:
   https://github.com/FundamentalFrequency/LMN-3-DAW/compare/v0.4.1...v0.4.2
```

Update the version in `CMakeLists.txt` e.g.

```diff
- project(LMN-3 VERSION 0.4.0)
+ project(LMN-3 VERSION 0.4.3)
```

Create a tag for the version:


```bash
git tag v0.4.3
git push --tags
```

Then [draft a new release](/releases/new), select the created tag then publish.
