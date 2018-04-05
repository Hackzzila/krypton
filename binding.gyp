{
  "includes": [
    "./config.gypi"
  ],
  "targets": [
     {
      'target_name': 'krypton',
      'cflags': [
        '-pthread',
        '-fno-exceptions',
        '-fno-strict-aliasing',
        '-Wall',
        '-Wno-unused-parameter',
        '-Wno-missing-field-initializers',
        '-Wextra',
        '-pipe',
        '-fno-ident',
        '-fdata-sections',
        '-ffunction-sections',
        '-fPIC',
        '-O3',
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
      ],
      'sources': [
        'src/main.cc',
        'src/Lame.cc',
        'src/Opus.cc',
        'src/PCM.cc',
        'src/Sodium.cc',
      ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_RTTI': 'YES',
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'MACOSX_DEPLOYMENT_TARGET':'10.10',
        'CLANG_CXX_LIBRARY': 'libc++',
        'CLANG_CXX_LANGUAGE_STANDARD':'c++11',
        'GCC_VERSION': 'com.apple.compilers.llvm.clang.1_0',
      },
    },
  ],
}