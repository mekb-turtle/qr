project('qr', 'c',
  version: '0.1',
  license: 'MPL-2.0',
  default_options: ['warning_level=3'])

main_name = 'qr'

libqr_d = 'src'/'libqr'
cli_d = 'src'/'cli'
test_d = 'src'/'test'

# define source files
src_all = files() # included for all the following
src_libqr = files(libqr_d/'qr.c', libqr_d/'qr.h', libqr_d/'bit_buffer.c', libqr_d/'bit_buffer.h', libqr_d/'qr_table.c', libqr_d/'qr_table.h', libqr_d/'utf8.c', libqr_d/'utf8.h', libqr_d/'gf256.c', libqr_d/'gf256.h', libqr_d/'qr_render.c', libqr_d/'qr_render.h')
src_cli = files(cli_d/'main.c', cli_d/'arg.c', cli_d/'arg.h', cli_d/'output.c', cli_d/'output.h', cli_d/'stbi_write.c', cli_d/'stbi_write.h', cli_d/'endian.c', cli_d/'endian.h')
src_test = files(test_d/'test.h') # included for tests only

# define test files, these get compiled separately
tests = [
  { 'files': files(test_d/'utf8.c') },
  { 'files': files(test_d/'utf8fail.c'), 'should_fail': true },
  { 'files': files(test_d/'bit_append.c') },
  { 'files': files(test_d/'bit_append_fail.c'), 'should_fail': true },
  { 'files': files(test_d/'endian.c'), 'src': files(cli_d/'endian.c', cli_d/'endian.h') },
  { 'files': files(test_d/'gf256.c') },
  { 'files': files(test_d/'qr_encode.c') },
  { 'files': files(test_d/'alignment.c') },
]

# define project metadata
url = 'https://github.com/mekb-turtle/qr'
name = meson.project_name()
version = meson.project_version()

dependencies_libqr = [
  dependency('iconv'),
]
dependencies_test = [

] + dependencies_libqr
dependencies_cli = [
]



# begin meson configuration

add_project_arguments(
  f'-DPROJECT_NAME="@name@"',
  f'-DPROJECT_VERSION="@version@"',
  f'-DPROJECT_URL="@url@"',
  language : 'c')

fs = import('fs')

# library
libqr = library('QR', sources: src_all + src_libqr, include_directories: include_directories(libqr_d), install: true, dependencies: dependencies_libqr)

# loop tests
foreach test_ : tests
  test_name = fs.stem(test_['files']) # basename without extension
  if test_.has_key('name')
    test_name = test_['name'] # custom test name
  endif

  extra_src = []
  if test_.has_key('src')
    extra_src = test_['src']
  endif

  test_exe = executable(test_name, sources: src_all + src_test + test_['files'] + extra_src, install: false, dependencies: dependencies_test, link_with: libqr)

  args = {}
  foreach key, val : test_
    # skip files and name
    if key != 'files' and key != 'name' and key != 'src'
      args += {key: val}
    endif
  endforeach

  test(test_name, test_exe, kwargs: args)
endforeach

# main executable
cli = executable(main_name, sources: src_all + src_cli, install: true, dependencies: dependencies_cli, link_with: libqr)
