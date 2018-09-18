rm -f ./build/*
emcc src/program.c -s EXPORTED_FUNCTIONS='["_driver","_free"]' -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["stringToUTF8", "Pointer_stringify","getValue"]' -s ENVIRONMENT=web -s MODULARIZE=1 -s EXPORT_ES6=1 -O3 -o build/out.js
wasm-opt ./build/*.wasm -O4 -c -o ./build/*.wasm
wasm-gc ./build/*.wasm
