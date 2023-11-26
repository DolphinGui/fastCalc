project('Fast Calculator', ['cpp'],  default_options : ['buildtype=debugoptimized', 'c_std=c11', 'cpp_std=c++23'])

add_project_arguments('-DPCRE2_CODE_UNIT_WIDTH=8', language : ['c', 'cpp'])
add_project_arguments('-DFCALC_FMT_FORMAT', language : ['c', 'cpp'])

cmake = import('cmake')

ctre_proj = cmake.subproject('ctre')
ctre = ctre_proj.dependency('ctre')

fmt = dependency('fmt', include_type : 'system')

fcalc = library('fcalc', ['src/fcalc.cpp'], 
    dependencies: [fmt, ctre], include_directories: include_directories(['include/fast_calc', 'include']))
calc = library('calc', ['src/calc.cpp'], 
    dependencies: [fmt, ctre], include_directories: include_directories('include/test', 'include'))

gbenchmark = dependency('benchmark', include_type : 'system')
gtest = dependency('gtest', include_type : 'system')
tcmalloc = dependency('libtcmalloc')

executable('calc', 'src/main.cpp',dependencies: [fmt, gbenchmark],
    include_directories : include_directories('include'), link_with: fcalc)

parse_bench = executable('parsing_benchmark', 'tests/parse_bench.cpp', dependencies: [fmt, gbenchmark, tcmalloc],
    include_directories : include_directories('include'), link_with: [fcalc, calc])
benchmark('parsing benchmark', parse_bench)

regex_test = executable('regex_test', 'tests/regex.cpp', dependencies: [fmt, gtest, ctre],
    include_directories : include_directories('include'), link_with: fcalc)
test( 'regex test', regex_test)
fcalc_test = executable('fcalc_test', 'tests/fcalc.cpp', dependencies: [fmt, gtest],
    include_directories : include_directories('include'), link_with: fcalc)
test('fcalc test', fcalc_test)
calc_test = executable('calc_test', 'tests/calc.cpp', dependencies: [fmt, gtest],
    include_directories : include_directories('include'), link_with: calc)
test('calc test', calc_test)