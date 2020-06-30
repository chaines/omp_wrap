{
    "targets": [
        {
            "target_name": "omp_wrap",
            "cflags!": [ "-fno-exceptions" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            "sources": [ 
                "OmpWrap.cc",
                "OMPEval/omp/CardRange.cpp",
                "OMPEval/omp/CombinedRange.cpp",
                "OMPEval/omp/EquityCalculator.cpp",
                "OMPEval/omp/HandEvaluator.cpp"
            ],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
        }
    ]
}