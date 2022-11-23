{
    "targets": [{
        "target_name": "saharaspice",
        "cflags!": ["-w", "-fpermissive", "-fno-exceptions"],
        "cflags_cc!": ["-fno-exceptions", "-fpermissive"],
        "sources": [
            "./src/libraries/saharaspice/log.cc",
            "./src/libraries/saharaspice/node.cc",
            "./src/libraries/saharaspice/global.cc",
            "./src/libraries/saharaspice/utils.cc",
            "./src/libraries/saharaspice/circuit.cc",
            "./src/libraries/saharaspice/ngspice.cc",
            "./src/libraries/saharaspice/i_ngspice.cc",
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")",
            "./src/libraries/saharaspice/",
            "./build_ngspice/include/"
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")",
        ],
        'defines': [],
        'conditions': [
            ['OS=="linux"', {
                'link_settings': {
                    'libraries': ['<!(pwd)/build_ngspice/lib/libngspice.a']
                },
            }],
            ['OS=="mac"', {
                'xcode_settings': {
                    'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                },
                'link_settings': {
                    'libraries': ['<!(pwd)/build_ngspice/lib/libngspice.a']
                },
            }]
        ]
    }]
}
