require 'mkmf'

#abort "missing pmNewContext()" unless have_func "pmNewContext"

asplode('pcp') unless find_library('pcp', 'pmNewContext')

dir_config("pcp/pcp_native")
create_makefile("pcp/pcp_native")