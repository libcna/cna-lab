# Third-party notices

`cna-lab` contains its own application source only.  It builds against adjacent
development checkouts and does not vendor their source trees.

- `../cnanext` provides CNA and is licensed under the Microsoft Public License
  (Ms-PL).  See `../cnanext/LICENSE`.
- `../sharp-runtimenext` provides Sharp Runtime and is licensed under the MIT
  License.  See `../sharp-runtimenext/LICENSE`.
- The OpenGLES3 configuration may fetch FNA3D through CNA's normal CMake
  dependency process.  Its notices remain with that dependency.

Build output copies only test/demo inputs: a glTF and XNB fixture from
`../cnanext`, a WAV fixture from CNA's audio example, and an MP4 fixture from
`../cna-examples`.  They are not committed to this repository.
