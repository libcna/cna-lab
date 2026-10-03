// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "CNA.Swift.Template",
    dependencies: [
        .package(path: "../cna-swift"),
    ],
    targets: [
        .executableTarget(
            name: "HelloGame",
            dependencies: [
                .product(name: "CNA", package: "cna-swift")
            ],
            path: "Sources"
        ),
    ]
)
