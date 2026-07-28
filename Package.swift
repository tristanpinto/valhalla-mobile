// swift-tools-version:5.8
import PackageDescription

// Use the local binary if true.
//
// Contour fork: back to upstream's default of false, now that this fork
// publishes its own xcframework. It briefly defaulted to true, which made the
// package build only on the one Mac that had the binary sitting in `build/`
// — a directory .gitignore excludes — so a clean checkout and every CI runner
// resolved a package they could not build.
let useLocalBinary = Context.environment["VALHALLA_MOBILE_DEV"].flatMap(Bool.init) ?? false

// Use the local binary
var binaryTarget: Target = .binaryTarget(
    name: "ValhallaWrapper",
    path: "build/apple/valhalla-wrapper.xcframework"
)

// This fork's own build, because upstream's 0.5.1 binary exposes only the
// `route` action — the whole reason the fork exists. Built for arm64 device
// and arm64 simulator by scripts/create_xcframework_apple_silicon.sh.
let version: String = "contour-0.5.1-actions"
let binaryURL: String =
    "https://github.com/tristanpinto/valhalla-mobile/releases/download/\(version)/valhalla-wrapper.xcframework.zip"
let binaryChecksum: String = "c3f1f06b58dd56982e339a972a6a2f62335afd157df22fb3ad90497f5b3ca732"

if !useLocalBinary {
    binaryTarget = .binaryTarget(
        name: "ValhallaWrapper",
        url: binaryURL,
        checksum: binaryChecksum
    )
}

let package = Package(
    name: "ValhallaMobile",
    platforms: [
        .iOS("16.4")
        // .tvOS(.v13),
        // .watchOS(.v6),
        // .macOS(.v10_13)
    ],
    products: [
        .library(
            name: "Valhalla",
            targets: ["Valhalla"]
        )
    ],
    dependencies: [
        .package(
            url: "https://github.com/Rallista/valhalla-openapi-models-swift.git",
            .upToNextMinor(from: "0.3.0")),
        .package(
            url: "https://github.com/UInt2048/Light-Swift-Untar.git", .upToNextMajor(from: "1.0.4")),
        .package(url: "https://github.com/apple/swift-docc-plugin", .upToNextMajor(from: "1.0.0")),
    ],
    targets: [
        .target(
            name: "Valhalla",
            dependencies: [
                "ValhallaObjc",
                "ValhallaWrapper",
                .product(name: "ValhallaConfigModels", package: "valhalla-openapi-models-swift"),
                .product(name: "ValhallaModels", package: "valhalla-openapi-models-swift"),
                .product(name: "Light-Swift-Untar", package: "Light-Swift-Untar"),
            ],
            path: "apple/Sources/Valhalla",
            resources: [
                .process("SupportData")
            ]
        ),
        .target(
            name: "ValhallaObjc",
            dependencies: ["ValhallaWrapper"],
            path: "apple/Sources/ValhallaObjc",
            linkerSettings: [.linkedLibrary("z")]
        ),
        binaryTarget,
        .testTarget(
            name: "ValhallaTests",
            dependencies: ["Valhalla"],
            path: "apple/Tests/ValhallaTests",
            resources: [.copy("TestData")]
        ),
    ],
    cLanguageStandard: .gnu17,
    cxxLanguageStandard: .cxx20
)
