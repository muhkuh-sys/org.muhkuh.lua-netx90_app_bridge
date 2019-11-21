local t = ...

t:install('doc/',  '${install_doc}/')
t:install('netx/', '${install_base}/netx/')
t:install('lua/',  '${install_lua_path}/')

t:install_dev('dev/include', '${install_dev_include}/')
t:install_dev('dev/lib',     '${install_dev_lib}/')

return true
