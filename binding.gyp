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
        'src/lame.cc',
        'src/opus.cc',
        'src/pcm.cc',
        'src/pcm-avx2.cc',
        'src/pcm-avx512.cc',
        'src/pcm-neon.cc',
        'src/sodium.cc',
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