#!/bin/bash

# Contour fork only. The upstream create_xcframework.sh packages three slices;
# this one packages the two Apple Silicon slices we actually build — an iPhone
# and an Apple Silicon simulator. The Intel simulator slice costs a third of
# the build time and is only needed by upstream CI, which builds it there.
#
# Not part of the upstream pull request.

set -e

libtool -static -o build/apple/arm64-ios/libvalhalla_all.a \
    build/apple/arm64-ios/install/lib/*.a

libtool -static -o build/apple/arm64-ios-simulator/libvalhalla_all.a \
    build/apple/arm64-ios-simulator/install/lib/*.a

rm -rf build/apple/valhalla-wrapper.xcframework

xcodebuild -create-xcframework \
    -library build/apple/arm64-ios/libvalhalla_all.a -headers build/apple/arm64-ios/install/include \
    -library build/apple/arm64-ios-simulator/libvalhalla_all.a -headers build/apple/arm64-ios/install/include \
    -output build/apple/valhalla-wrapper.xcframework
