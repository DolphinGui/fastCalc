project('Fast Calculator', ['cpp'],  default_options : ['buildtype=debugoptimized', 'c_std=c11', 'cpp_std=c++23'])

add_project_arguments('-DPCRE2_CODE_UNIT_WIDTH=8', language : ['c', 'cpp'])
add_project_arguments('-DFCALC_FMT_FORMAT', language : ['c', 'cpp'])

pcre = dependency('libpcre2-8', include_type : 'system')

fmt = dependency('fmt', include_type : 'system')

fcalc = library('fcalc', ['src/fcalc.cpp', 'src/re.cpp'], 
    dependencies: [pcre, fmt], include_directories: include_directories('include/fast_calc'))

gbenchmark = dependency('benchmark', include_type : 'system')
gtest = dependency('gtest', include_type : 'system')

executable('calc', 'src/main.cpp',dependencies: [fmt, gbenchmark],
    include_directories : include_directories('include'), link_with: fcalc)

parse_bench = executable('parsing_benchmark', 'benchmarks/parse.cpp', dependencies: [fmt, gbenchmark],
    include_directories : include_directories('include'), link_with: fcalc)
benchmark('parsing benchmark', parse_bench)

regex_test = executable('regex_test', 'tests/regex.cpp', dependencies: [fmt, gtest],
    include_directories : include_directories('include'), link_with: fcalc)
test( 'regex test', regex_test)
fcalc_test = executable('fcalc_test', 'tests/fcalc.cpp', dependencies: [fmt, gtest],
    include_directories : include_directories('include'), link_with: fcalc)
test('calc test', fcalc_test)