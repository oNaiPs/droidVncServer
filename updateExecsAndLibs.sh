#!/bin/bash

cd $(dirname $0)

for i in libs/*; do
  if [[ -d $i && -e $i/androidvncserver ]];then
    echo Moving $i/androidvncserver to $i/libandroidvncserver.so;
    mv $i/androidvncserver $i/libandroidvncserver.so;
  fi
done
cp -frv nativeMethods/libs/* libs
echo Done.
