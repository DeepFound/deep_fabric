import os
import shutil
import glob

try:
    from setuptools import setup
    from setuptools.command.install import install

except ImportError:
    from distutils.core import setup
    from distutils.command.install import install

################################################################################

class issue(Exception):
    def __init__(self, errorStr):
        self.errorStr = errorStr
    def __str__(self):
        return repr(self.errorStr)

################################################################################

class post_install(install):
    def copyStuff(self, destDir, srcDir, pattern='*'):
        if os.path.isdir(destDir) == False:
            os.makedirs(destDir)

        _bsDir = os.path.abspath(srcDir)

        if os.path.exists(_bsDir) == False:
            raise issue('No files at: ' % (_bsDir,))

        _files = glob.glob(_bsDir + '/' + pattern)
        for _file in _files:
            print 'copying %s -> %s' % (_file, destDir,)
            shutil.copy2(_file, destDir)

    def run(self):
        install.run(self)

        _sqlDestDir = '/usr/local/deep-demo/'
        _sqlSrcDir  = './scripts/demo/'

        self.copyStuff(_sqlDestDir, _sqlSrcDir, '*.sql')

################################################################################

setup(name         = 'deep-test-tools',
      description  = 'Deep Test Tools',
      author       = 'Vincent King/Eric Mann/Grant Mills',
      author_email = 'vincent@deep.is, eric@deep.is, grant@deep.is',
      url          = 'http://deepis.com',
      version      = '0.1',

      cmdclass     = {'install': post_install},

      packages     = ['scripts.maestro',
                      ],
      scripts      = ['scripts/transmute_sql/transmute_sql.py',
                      'binaries/deepis-fabric',
                      'scripts/RealtimeFabricClient.py',
                      'scripts/demo/demo.sh',
                      'scripts/demo/demo2.sh',
                      'scripts/demo/makeInserts.py',
                      ],
      install_requires = ['pyparsing',],

      )

################################################################################
