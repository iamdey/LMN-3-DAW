# Release procedure

_(This document is meant for maintainers of this repository.)_

- Make sure it's all green.
- The ./CHANGELOG.md file is filled

Choose the correct semantic version according to the changes.
Set manually the version and the date of the release in the CHANGELOG file. e.g.

```
## 0.4.3 - 1970/1/1
```

Create a tag for the version:


```bash
git tag v0.4.3
git push --tags
```

Then [draft a new release](/releases/new), select the created tag then publish.
