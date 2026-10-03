// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The messages the buffer family raises.
    ///
    /// Every one is read out of the embedded string table of the registered
    /// `Microsoft.Xna.Framework.dll` and pinned in
    /// `tools/api_compat/reference/xna40-selected-resource-strings.json`, where
    /// the assembly audit compares it against the binary on every run. None is
    /// transcribed from documentation.
    internal enum BufferResources {
        /// `FrameworkResources.MustBeValidIndex`.
        static let mustBeValidIndex =
            "This parameter must be a valid index within the array."

        /// `FrameworkResources.VertexStrideTooSmall`.
        static let vertexStrideTooSmall =
            "The vertex stride is too small for the type of data requested. "
            + "This is not allowed."

        /// `FrameworkResources.ResourceDataMustBeCorrectSize`.
        static let resourceDataMustBeCorrectSize =
            "The array is not the correct size for the amount of data requested."

        /// `FrameworkResources.WriteOnlyGetNotSupported`.
        static let writeOnlyGetNotSupported =
            "Calling GetData on a resource that was created with "
            + "BufferUsage.WriteOnly is not supported."

        /// `FrameworkResources.IndexBuffersMustBeSizedCorrectly`.
        static let indexBuffersMustBeSizedCorrectly =
            "IndexBuffers may be created only with types that are 16 bit or "
            + "32 bit in size."

        /// `FrameworkResources.VertexTypeNotValueType`, a one-argument format.
        static let vertexTypeNotValueType =
            "Invalid vertex type. {0} is not a value type."

        /// `FrameworkResources.VertexTypeNotIVertexType`, a one-argument format.
        static let vertexTypeNotIVertexType =
            "Invalid vertex type. {0} does not implement the IVertexType interface."

        /// `FrameworkResources.VertexTypeWrongSize`, a one-argument format.
        static let vertexTypeWrongSize =
            "Invalid vertex type. The size of {0} does not match the stride of "
            + "its vertex declaration."

        /// `String.Format(CultureInfo.CurrentCulture, template, vertexType)`.
        ///
        /// All three vertex-type messages format one `System.Type` into a
        /// single `{0}`, and the CLR formats a `Type` with its `ToString()`,
        /// which is the type's full name. None of the three is
        /// culture-sensitive: the only substitution is a type name.
        static func formatted(_ template: String, _ type: Any.Type) -> String {
            template.replacingOccurrences(
                of: "{0}",
                with: Microsoft.Xna.Framework.Graphics.GraphicsResource
                    .clrTypeName(ofType: type))
        }
    }

    /// `Helpers.ValidateCopyParameters(Int32 dataLength, Int32 dataIndex,
    /// Int32 elementCount)`, shared by both buffer families and by
    /// `GraphicsDevice`'s user-primitive draws.
    ///
    /// ```text
    /// if (dataIndex < 0 || dataIndex > dataLength)
    ///     throw new ArgumentOutOfRangeException("dataIndex", MustBeValidIndex);
    /// if (elementCount + dataIndex > dataLength)
    ///     throw new ArgumentOutOfRangeException("elementCount", MustBeValidIndex);
    /// if (elementCount <= 0)
    ///     throw new ArgumentOutOfRangeException("elementCount", MustBeValidIndex);
    /// ```
    ///
    /// The first name is **`dataIndex`**, not the caller's `startIndex`: the
    /// helper is shared and reports its own parameter, which is what reaches
    /// the caller. The order matters as much as the names — an index past the
    /// end of the array is blamed on `dataIndex`, and only a *window* past the
    /// end is blamed on `elementCount`.
    internal static func validateCopyParameters(
        dataLength: Int, dataIndex: Int32, elementCount: Int32
    ) throws {
        if dataIndex < 0 || Int(dataIndex) > dataLength {
            throw CNAArgumentOutOfRangeException(
                paramName: "dataIndex", message: BufferResources.mustBeValidIndex)
        }
        if Int(elementCount) + Int(dataIndex) > dataLength {
            throw CNAArgumentOutOfRangeException(
                paramName: "elementCount", message: BufferResources.mustBeValidIndex)
        }
        if elementCount <= 0 {
            throw CNAArgumentOutOfRangeException(
                paramName: "elementCount", message: BufferResources.mustBeValidIndex)
        }
    }
}
