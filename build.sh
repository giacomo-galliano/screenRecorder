#!/bin/bash

echo "Starting building process.."

mkdir build
if[ $? -eq 0 ]
then
  echo "./build directory created succesfully"
else
  echo "Error. Unable to create ./build directory"

cd build
echo "Inside ./build directory"

cmake ..
make
if[ $? -eq 0 ]
then
  echo "build completed successfully"
else
  echo "Error. Unable to build correctly"