rm -f ./build/*
emcc src/main.c -s EXPORTED_FUNCTIONS='["_foo"]' -s ENVIRONMENT=web -s MODULARIZE=1 -s EXPORT_ES6=1 -O3 -o build/out.js
wasm-opt ./build/*.wasm -O4 -c -o ./build/*.wasm
wasm-gc ./build/*.wasm
