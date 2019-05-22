#!/usr/bin/env python

try:
  import sys
  import os.path
  from   helpers.io import ErrorMessage
  from   helpers.io import WarningMessage
  import Viewer
except ImportError, e:
  serror = str(e)
  if serror.startswith('No module named'):
    module = serror.split()[-1]
    print '[ERROR] The <%s> python module or symbol cannot be loaded.' % module
    print '        Please check the integrity of the <coriolis> package.'
  if str(e).find('cannot open shared object file'):
    library = serror.split(':')[0]
    print '[ERROR] The <%s> shared library cannot be loaded.' % library
    print '        Under RHEL 6, you must be under devtoolset-2.'
    print '        (scl enable devtoolset-2 bash)'
  sys.exit(1)
except Exception, e:
  print '[ERROR] A strange exception occurred while loading the basic Coriolis/Python'
  print '        modules. Something may be wrong at Python/C API level.\n'
  print '        %s' % e
  sys.exit(2)


def katanaHook ( **kw ):
    katana = None
    if kw.has_key('katana'):
        katana = kw['katana']
    else:
        print ErrorMessage( 3, 'katanaHook(): Must be run from a KatanaEngine.' )
        return

    userInit = os.path.join( os.getcwd(), '.coriolis2/katana.py' )
    if (os.path.exists(userInit)):
      execfile( userInit )
    return
