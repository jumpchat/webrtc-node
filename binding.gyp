{
    'targets': [
        {
            'target_name': '<(module_name)',
            'product_dir': '<(module_path)',
            "sources": [
                "src/DesktopCapturer.cpp",
                "src/EventEmitter.cpp",
                "src/GetSources.cpp",
                "src/MediaAudioRenderer.cpp",
                "src/MediaConstraints.cpp",
                "src/MediaDevices.cpp",
                "src/MediaStream.cpp",
                "src/MediaStreamTrack.cpp",
                "src/MediaVideoRenderer.cpp",
                "src/Module.cpp",
                "src/Platform.cpp",
                "src/ScreencastCapturer.cpp",
                # "src/BackTrace.cc",
                # "src/DataChannel.cc",
                # "src/Observers.cc",
                # "src/PeerConnection.cc",
                # "src/Stats.cc",
            ],
            "include_dirs": [
                "./src",
                "<!(node scripts/constants.js WEBRTC_INCLUDE_DIR)",
                "<!(node -e \"require('nan')\")"
            ],
            "link_settings": {
                "conditions": [
                    ['OS=="mac"', {
                        'libraries': [
                            '-framework Foundation',
                            '-framework CoreAudio',
                            '-framework CoreMedia',
                            '-framework CoreVideo',
                            '-framework CoreGraphics',
                            '-framework AudioToolbox',
                            '-framework AVFoundation',
                            '-framework AppKit',
                        ],
                        "configurations": {
                            "Debug": {
                                "xcode_settings": {
                                    "OTHER_LDFLAGS": [
                                        '<!(node scripts/constants.js WEBRTC_LIB_DEBUG)'
                                    ]
                                }
                            },
                            "Release": {
                                "xcode_settings": {
                                    "OTHER_LDFLAGS": [
                                        '<!(node scripts/constants.js WEBRTC_LIB_RELEASE)'
                                    ]
                                }
                            }
                        },
                    }],
                    ['OS=="linux"', {
                        'libraries': [
                            "-lX11",
                        ],
                        "configurations": {
                            "Debug": {
                                "ldflags": [
                                    '<!(node scripts/constants.js WEBRTC_LIB_DEBUG)'
                                ]
                            },
                            "Release": {
                                "ldflags": [
                                    '<!(node scripts/constants.js WEBRTC_LIB_RELEASE)'
                                ]
                            }
                        },
                    }],
                    ['OS=="win"', {
                        'libraries': [
                            'dmoguids.lib',
                            'msdmo.lib',
                            'secur32.lib',
                            'winmm.lib',
                            'wmcodecdspuuid.lib',
                            'ws2_32.lib',
                            'dmoguids.lib',
                            'amstrmid.lib',
                            'd3d11.lib',
                            'dxgi.lib'
                        ],
                        "configurations": {
                            "Debug": {
                                "msvs_settings": {
                                    "VCLinkerTool": {
                                        "AdditionalDependencies": [
                                            '<!(node scripts/constants.js WEBRTC_LIB_DEBUG)'
                                        ]
                                    }
                                }
                            },
                            "Release": {
                                "msvs_settings": {
                                    "VCLinkerTool": {
                                        "AdditionalDependencies": [
                                            '<!(node scripts/constants.js WEBRTC_LIB_RELEASE)'
                                        ]
                                    }
                                }
                            }
                        },
                    }]
                ]
            },
            'xcode_settings': {
                'MACOSX_DEPLOYMENT_TARGET': '10.9',
                'OTHER_CFLAGS': [
                    '-fvisibility=hidden',
                    '-fvisibility-inlines-hidden',
                ]
            },
            'conditions': [
                ['OS=="linux"', {
                    'defines': [
                        'WEBRTC_LINUX',
                        'WEBRTC_POSIX=1',
                        'USE_BUILTIN_SW_CODECS',
                    ],
                }],
                ['OS=="mac"', {
                    'defines': [
                        'WEBRTC_MAC',
                        'WEBRTC_POSIX=1',
                        'USE_BUILTIN_SW_CODECS',
                    ],
                }],
                ['OS=="win"', {
                    'defines': [
                        'WEBRTC_WIN',
                        'NOGDI',
                        'NOMINMAX',
                        'USE_BUILTIN_SW_CODECS',
                    ],
                }],
            ],
            # 'dependencies': [ 'run_build_script' ],
            #'hard_dependency': 1,
            #'type': 'none',

            # 'actions': [
            #     {
            #         'action_name': 'run_build_script',
            #         'inputs': [],
            #         'outputs': [
            #             '<!(node scripts/constants.js WEBRTC_LIB)',
            #         ],
            #         'action': [
            #             'node', 'scripts/download_libwebrtc.js'
            #         ],
            #     },
            # ]
        },
    ],
    'target_defaults': {
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
                'defines': [ 'DEBUG', '_DEBUG' ],
            },
            'Release': {
                'defines': [ 'NDEBUG' ],
            }
        }
    }
}
