cd c && ./release.sh
cd -
cp ./c/build/* ./src
yarn build || npm build
cp src/out* dist/
cp -r highlight ./dist/
mkdir ./dist/src
cp ./c/src/* ./dist/src
