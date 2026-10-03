// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// The payload a projected CLR/XNA exception must carry, asserted whole.
///
/// Every one of the four facts is separately observable and separately
/// wrong-able: the class decides which `catch` sees the failure, `Message` is
/// composed rather than stored, `ParamName` is nil unless the constructor the
/// raise site selected supplied one, and `HResult` distinguishes classes that
/// otherwise look alike. Asserting only the message — which is what the layer
/// did before these classes existed — would pass on every one of them.
func assertProjected<Thrown: CNAException>(
    _ expected: Thrown.Type,
    message: String,
    paramName: String? = nil,
    hResult: Int32,
    file: StaticString = #filePath,
    line: UInt = #line,
    _ body: () throws -> Void
) {
    do {
        try body()
        XCTFail("expected \(expected) but nothing was thrown", file: file, line: line)
    } catch let error as CNAException {
        let actual = Swift.type(of: error)
        XCTAssertTrue(
            actual == expected,
            "threw \(actual) rather than \(expected)", file: file, line: line)
        XCTAssertEqual(error.Message, message, file: file, line: line)
        XCTAssertEqual(
            (error as? CNAArgumentException)?.ParamName, paramName,
            "ParamName", file: file, line: line)
        XCTAssertEqual(error.HResult, hResult, "HResult", file: file, line: line)
    } catch {
        XCTFail(
            "threw \(error), which is not a CNAException at all",
            file: file, line: line)
    }
}

/// The `Message` an `ArgumentException` family member composes.
///
/// `ArgumentException.get_Message` appends `Arg_ParamName_Name`, formatted
/// with the parameter name, to the base message around `Environment.NewLine`,
/// which the admitted `mscorlib` declares as the IL literal `"\r\n"`.
func composedArgumentMessage(_ base: String, paramName: String) -> String {
    base + "\r\n" + "Parameter name: " + paramName
}
