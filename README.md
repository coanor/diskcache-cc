# intro

This is a rewrite implementation of exist [diskcache](https://github.com/GuanceCloud/cliutils/tree/main/diskcache) in Golang. I personally rewrite it to learn modern C++.

## Status

- [x] Defined basic class `disk_cache`
- [ ] Implementing various class methods 
- [ ] Add basic unit test and reach coverage to %80+
  - [x] Add basic google test
- [ ] Add Prometheus metrics
- [ ] Add CI/CD
- [ ] Add basic network `Put()/Get()`(and the new feature that missing in [oginal implements](https://github.com/GuanceCloud/cliutils/tree/main/diskcache))
- [ ] Make some benchmark vs Golang verison
- [x] Add cmake support
- [x] Add logging support(spdlog)
