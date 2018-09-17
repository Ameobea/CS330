cd c && ./release.sh
cd -
cp ./c/build/* ./src
yarn build || npm build
cp src/out* dist/
