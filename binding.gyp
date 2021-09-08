{
    "targets": [
        {
            "target_name": "basil",
            "target_arch": "win32",
            "cflags_cc": [
                "-std=c++17",
                "-fexceptions"
            ],
            "sources": [
                "lib/basil-export.cc"
            ],
            'include_dirs': [
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'defines': [
                "NAPI_CPP_EXCEPTIONS"
            ],
            "msvs_configuration_platform": "Win32",
            "msbuild_settings": {
                "ClCompile": {
                    "LanguageStandard": "stdcpp17"
                }
            }
        }
    ]
}