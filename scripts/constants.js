var path = require('path')

var HOME = process.env['HOME'] || path.resolve(__dirname, '..');
var WEBRTC_VERSION = '2.2.0'
var WEBRTC_REVISION = '4ab51cb6d7d4530b344e63ed8274d1454b7a5f39';
var WEBRTC_ROOT_DIR = path.resolve(HOME, '.webrtc');
var DEPOT_TOOLS_REPO = 'https://chromium.googlesource.com/chromium/tools/depot_tools.git';
var DEPOT_TOOLS_DIR = path.resolve(WEBRTC_ROOT_DIR, 'depot_tools');
var WEBRTC_DIR = path.resolve(WEBRTC_ROOT_DIR, 'webrtc', WEBRTC_REVISION);
var WEBRTC_OS = 'none';
var WEBRTC_LIBNAME_DEBUG = 'libwebrtcd.a';
var WEBRTC_LIBNAME_RELEASE = 'libwebrtc.a';
switch (process.platform) {
    case 'win32':
        WEBRTC_OS = 'win';
        WEBRTC_LIBNAME_DEBUG  = 'webrtcd.lib';
        WEBRTC_LIBNAME_RELEASE  = 'webrtc.lib';
        break;
    case 'darwin':
        WEBRTC_OS = 'mac';
        break;
    case 'linux':
        WEBRTC_OS = 'linux';
        break;
}
var WEBRTC_CPU = process.arch;
var WEBRTC_LIB_DIR = path.resolve(WEBRTC_DIR, 'lib', WEBRTC_OS, WEBRTC_CPU)
var WEBRTC_LIB_DEBUG = path.resolve(WEBRTC_LIB_DIR, WEBRTC_LIBNAME_DEBUG)
var WEBRTC_LIB_RELEASE = path.resolve(WEBRTC_LIB_DIR, WEBRTC_LIBNAME_RELEASE)
var WEBRTC_INCLUDE_DIR = path.resolve(WEBRTC_DIR, 'include')
var WEBRTC_LIB_URL = [
    'https://bitbucket.org/jumpchat/webrtc-lib/downloads/libwebrtc-',
        WEBRTC_OS, '-', WEBRTC_CPU, '-', WEBRTC_VERSION,'.tar.xz'
].join('')

module.exports = {
    'WEBRTC_ROOT_DIR': WEBRTC_ROOT_DIR,
    'WEBRTC_REVISION': WEBRTC_REVISION,
    'WEBRTC_DIR': WEBRTC_DIR,
    'WEBRTC_LIB_DIR': WEBRTC_LIB_DIR,
    'WEBRTC_LIB_DEBUG': WEBRTC_LIB_DEBUG,
    'WEBRTC_LIB_RELEASE': WEBRTC_LIB_RELEASE,
    'WEBRTC_LIB_URL': WEBRTC_LIB_URL,
    'WEBRTC_INCLUDE_DIR': WEBRTC_INCLUDE_DIR,
    'WEBRTC_REPO': 'https://chromium.googlesource.com/external/webrtc.git',
    'DEPOT_TOOLS_DIR': DEPOT_TOOLS_DIR,
    'DEPOT_TOOLS_REPO': DEPOT_TOOLS_REPO
}

if (require.main === module) {
    var arg = process.argv[process.argv.length-1]
    if (arg) {
        console.log(module.exports[arg])
    }
}
