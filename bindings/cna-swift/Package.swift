// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "CNA",
    products: [
        .library(name: "CNA", targets: ["CNA"]),
    ],
    targets: [
        .target(
            name: "CNAShim",
            path: "Sources/CNAShim",
            publicHeadersPath: "include"
        ),
        .target(
            name: "CNA",
            dependencies: ["CNAShim"],
            path: "Sources/CNA",
            linkerSettings: [
                .linkedLibrary("dl", .when(platforms: [.linux])),
            ]
        ),
        .testTarget(
            name: "CNATests",
            dependencies: ["CNA"],
            path: "Tests/CNATests"
        ),
    ]
)
