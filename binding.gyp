{
  "targets": [
    {
      "target_name": "jumpblock-impl",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [
        # tree-sitter
        "third-party/tree-sitter/lib/src/lib.c",
        # tree-sitter-cpp
        "third-party/tree-sitter-cpp/src/parser.c",
        "third-party/tree-sitter-cpp/src/scanner.c",
        # tree-sitter-python
        "third-party/tree-sitter-python/src/parser.c",
        "third-party/tree-sitter-python/src/scanner.c",
        # my-code
        "jumpblock-impl.cc"
      ],
      "include_dirs": [
         "<!@(node -p \"require('node-addon-api').include\")",
        "third-party/tree-sitter/lib/src",
        "third-party/tree-sitter/lib/include"
      ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
    }
  ]
}
