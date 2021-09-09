{
    "targets": [
        {
            "target_name": "basil",
            "cflags_cc": [
                "-std=c++latest"
            ],
            "sources": [
                "basil/basil.cc",
                "lib/basil-export.cc"
            ],
            'include_dirs': [
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'defines': [
                "NAPI_CPP_EXCEPTIONS"
            ],
            "msbuild_settings": {
                "ClCompile": {
                    "LanguageStandard": "stdcpplatest"
                }
            }
        }
    ]
}