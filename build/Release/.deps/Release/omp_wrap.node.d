cmd_Release/omp_wrap.node := ln -f "Release/obj.target/omp_wrap.node" "Release/omp_wrap.node" 2>/dev/null || (rm -rf "Release/omp_wrap.node" && cp -af "Release/obj.target/omp_wrap.node" "Release/omp_wrap.node")
