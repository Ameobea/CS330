rm -f ./build/*
emcc src/program.c -s EXPORTED_FUNCTIONS='["_driver","_free"]' -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["stringToUTF8","Pointer_stringify"]' -s ENVIRONMENT=web -s MODULARIZE=1 -s EXPORT_ES6=1 -O3 -o build/out.js
wasm-gc ./build/*.wasm
wasm-opt ./build/*.wasm -O4 -c -o ./build/*.wasm
