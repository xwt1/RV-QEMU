project('plugin_test', 'c')

glib_dep = dependency('glib-2.0')
t = []
foreach i : ['ins_print','execlog']
  t += shared_module(i, files(i + '.c'),
                     include_directories: ['../include/qemu',
                                          '../include',
                                          '../include/exec'],
                     dependencies: [glib_dep])
endforeach
alias_target('test-plugins', t)

