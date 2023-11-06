project('Fast Calculator', ['cpp'],  default_options : ['buildtype=debugoptimized', 'c_std=c11', 'cpp_std=c++23'])

cmake = import('cmake')

fcalc = library('libfcalc', 'src/fcalc.cpp', include_directories: include_directories('include/fast_calc'))

gbenchmark = dependency('benchmark')
fmt = dependency('fmt')

executable('calc', 'src/main.cpp',dependencies: [fmt, gbenchmark], include_directories : include_directories('include'), link_with: fcalc)

parse_bench = executable('parsing_benchmark', 'benchmarks/parse.cpp', dependencies: [fmt, gbenchmark],
    include_directories : include_directories('include'), link_with: fcalc)
benchmark('parsing benchmark', parse_bench)
