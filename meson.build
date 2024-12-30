project('qr', 'c',
  version: '0.1',
  license: 'MPL-2.0',
  default_options: ['warning_level=3'])

main_name = 'qr'

# define source files
src_main = files('src'/'main.c') # excluded for tests
src = files('src'/'qr.c', 'src'/'qr.h', 'src'/'utf8.c', 'src'/'utf8.h', 'src'/'arg.c', 'src'/'arg.h', 'src'/'output.c', 'src'/'output.h', 'src'/'util.c', 'src'/'util.h', 'src'/'qr_table.c', 'src'/'qr_table.h', 'src'/'stbi_write.c', 'src'/'stbi_write.h')
src_test = files() # included for tests only

# define test files, these get compiled separately
tests = [
  { 'files': files('test'/'utf8.c') },
  { 'files': files('test'/'utf8fail.c'), 'should_fail': true },
  { 'files': files('test'/'bit_append.c') },
  { 'files': files('test'/'bit_append_fail.c'), 'should_fail': true },
  { 'files': files('test'/'endian.c') },
]

# add custom binary/text files to be compiled into the binary
bin = files()

# define project metadata
url = 'https://github.com/mekb-turtle/qr'
name = meson.project_name()
version = meson.project_version()

dependencies = [
  dependency('iconv'),
]



# begin meson configuration

add_project_arguments(
  f'-DPROJECT_NAME="@name@"',
  f'-DPROJECT_VERSION="@version@"',
  f'-DPROJECT_URL="@url@"',
  language : 'c')

# loop binary files
foreach bin_file : bin
  xxd = find_program('xxd')
  src += custom_target(input: bin_file, command: [
      xxd, '-i', '-n', '@PLAINNAME@', '@INPUT@', '@OUTPUT@'
    ],
    output: '@PLAINNAME@.c', install: false, build_by_default: true)
endforeach

fs = import('fs')

# loop tests
foreach test_ : tests
  test_name = fs.stem(test_['files']) # basename without extension
  if test_.has_key('name')
    test_name = test_['name'] # custom test name
  endif

  test_exe = executable(test_name, sources: src + src_test + test_['files'], install: false, dependencies: dependencies)

  args = {}
  foreach key, val : test_
    # skip files and name
    if key != 'files' and key != 'name'
      args += {key: val}
    endif
  endforeach

  test(test_name, test_exe, kwargs: args)
endforeach

# main executable
exe = executable(main_name, sources: src + src_main, install: true, dependencies: dependencies)
