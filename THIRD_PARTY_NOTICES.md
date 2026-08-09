# Third-party notices

This file preserves notices for sources that Myra-CNA plans to translate or
reuse. Inclusion here does not assert that the corresponding source or asset
has already been copied into this repository; `UPSTREAM_MANIFEST.md` is the
authoritative file-level record.

## Myra

Source: [MyraUI/Myra](https://github.com/MyraUI/Myra), revision
`0d79b939310bfe1d00b21803fe15e291caf60aa1`.

MIT License

Copyright (c) 2017-2020 The Myra Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## MonoGame.Extended lineage in Myra

Myra's `Utility/InputExtension.cs` and `Graphics2D/RenderContext.Shapes.cs`
state that they were borrowed from MonoGame.Extended. Any direct translation
of those files preserves this notice in its source header and manifest row.

The MIT License (MIT)

Copyright (c) 2015 Dylan Wilson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## MonoGame lineage in Myra

Myra's `Graphics2D/Transform.cs` explicitly says that its transform-building
algorithm was borrowed from MonoGame's `SpriteBatch.DrawString`. Myra's
`Utility/CurrentPlatform.cs` and FNA-excluded `Platform/Keys.cs` also carry an
explicit MonoGame Team copyright header referring to MonoGame's licence. The
official MonoGame repository states that the project is under the Microsoft
Public License (Ms-PL), except where separately noted.

This Myra-CNA port retains that attribution in `Transform.hpp` and the manifest.
It does not translate `CurrentPlatform.cs` or `Platform/Keys.cs`: CNA's platform
APIs replace the former and the selected FNA build excludes the latter. The
exact historical MonoGame revision from which Myra borrowed these sources has
not been established and remains a release audit item; no additional whole
MonoGame source file is copied directly into this repository. See
`docs/source-lineage-audit.md`.

Microsoft Public License (Ms-PL)

This license governs use of the accompanying software. If you use the software,
you accept this license. If you do not accept the license, do not use the software.

1. Definitions

The terms "reproduce," "reproduction," "derivative works," and "distribution"
have the same meaning here as under U.S. copyright law. A "contribution" is the
original software, or any additions or changes to the software. A "contributor"
is any person that distributes its contribution under this license. "Licensed
patents" are a contributor's patent claims that read directly on its
contribution.

2. Grant of Rights

(A) Copyright Grant - Subject to the terms of this license, including the
license conditions and limitations in section 3, each contributor grants you a
non-exclusive, worldwide, royalty-free copyright license to reproduce its
contribution, prepare derivative works of its contribution, and distribute its
contribution or any derivative works that you create.

(B) Patent Grant - Subject to the terms of this license, including the license
conditions and limitations in section 3, each contributor grants you a
non-exclusive, worldwide, royalty-free license under its licensed patents to
make, have made, use, sell, offer for sale, import, and/or otherwise dispose of
its contribution in the software or derivative works of the contribution in the
software.

3. Conditions and Limitations

(A) No Trademark License - This license does not grant you rights to use any
contributors' name, logo, or trademarks.

(B) If you bring a patent claim against any contributor over patents that you
claim are infringed by the software, your patent license from such contributor
to the software ends automatically.

(C) If you distribute any portion of the software, you must retain all
copyright, patent, trademark, and attribution notices that are present in the
software.

(D) If you distribute any portion of the software in source code form, you may
do so only under this license by including a complete copy of this license with
your distribution. If you distribute any portion of the software in compiled or
object code form, you may only do so under a license that complies with this
license.

(E) The software is licensed "as-is." You bear the risk of using it. The
contributors give no express warranties, guarantees, or conditions. You may have
additional consumer rights under your local laws which this license cannot
change. To the extent permitted under your local laws, the contributors exclude
the implied warranties of merchantability, fitness for a particular purpose and
non-infringement.

## TextCopy lineage in Myra

Myra includes TextCopy-derived clipboard sources. If their implementation is
translated rather than replaced by CNA Clipboard, the corresponding source
must carry this notice and a manifest row.

MIT License

Copyright (c) 2018 Simon Cropp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## FontStashSharp (audited; not incorporated)

Sources audited for the prospective font compatibility work:

- [FontStashSharp 1.5.6](https://github.com/FontStashSharp/FontStashSharp/tree/1.5.6),
  commit `24f3dc46d59dcda0dddb59754a99aeaefc9bd369`;
- [FontStashSharp.Base 1.2.3](https://github.com/FontStashSharp/FontStashSharp.Base/commit/670ecb8c7a323fcce6d9c176fa3692b4dd28b959),
  commit `670ecb8c7a323fcce6d9c176fa3692b4dd28b959`.

No FontStashSharp source, binary, or package has been incorporated yet. This
notice is recorded before the explicit dependency decision required by P3-004.

The zlib/libpng License

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use
of this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject
to the following restrictions:

The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an
acknowledgment in the product documentation would be appreciated but is not
required.

Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

This notice may not be removed or altered from any source distribution.

## Inter font (audited; not incorporated)

Myra's `Inter-Regular.ttf` has SHA-256
`1041a8cf17dab7579acef0cc46b21f6497ec1ae01918ddc3495416efb81a4780`.
Its metadata identifies Inter 3.012, copyright 2020 The Inter Project Authors,
and build source commit
[`06b166889e335a2454c0767734a05b27f6403098`](https://github.com/rsms/inter/commit/06b166889e335a2454c0767734a05b27f6403098).
The source revision's notice follows. No font file has been incorporated yet;
see `docs/default-assets-audit.md`.

Copyright (c) 2016-2019 The Inter Project Authors (me@rsms.me)

This Font Software is licensed under the SIL Open Font License, Version 1.1.
This license is copied below, and is also available with a FAQ at:
http://scripts.sil.org/OFL

-----------------------------------------------------------
SIL OPEN FONT LICENSE Version 1.1 - 26 February 2007
-----------------------------------------------------------

PREAMBLE
The goals of the Open Font License (OFL) are to stimulate worldwide
development of collaborative font projects, to support the font creation
efforts of academic and linguistic communities, and to provide a free and
open framework in which fonts may be shared and improved in partnership
with others.

The OFL allows the licensed fonts to be used, studied, modified and
redistributed freely as long as they are not sold by themselves. The
fonts, including any derivative works, can be bundled, embedded,
redistributed and/or sold with any software provided that any reserved
names are not used by derivative works. The fonts and derivatives,
however, cannot be released under any other type of license. The
requirement for fonts to remain under this license does not apply
to any document created using the fonts or their derivatives.

DEFINITIONS
"Font Software" refers to the set of files released by the Copyright
Holder(s) under this license and clearly marked as such. This may
include source files, build scripts and documentation.

"Reserved Font Name" refers to any names specified as such after the
copyright statement(s).

"Original Version" refers to the collection of Font Software components as
distributed by the Copyright Holder(s).

"Modified Version" refers to any derivative made by adding to, deleting,
or substituting -- in part or in whole -- any of the components of the
Original Version, by changing formats or by porting the Font Software to a
new environment.

"Author" refers to any designer, engineer, programmer, technical
writer or other person who contributed to the Font Software.

PERMISSION AND CONDITIONS
Permission is hereby granted, free of charge, to any person obtaining
a copy of the Font Software, to use, study, copy, merge, embed, modify,
redistribute, and sell modified and unmodified copies of the Font
Software, subject to the following conditions:

1) Neither the Font Software nor any of its individual components,
in Original or Modified Versions, may be sold by itself.

2) Original or Modified Versions of the Font Software may be bundled,
redistributed and/or sold with any software, provided that each copy
contains the above copyright notice and this license. These can be
included either as stand-alone text files, human-readable headers or
in the appropriate machine-readable metadata fields within text or
binary files as long as those fields can be easily viewed by the user.

3) No Modified Version of the Font Software may use the Reserved Font
Name(s) unless explicit written permission is granted by the corresponding
Copyright Holder. This restriction only applies to the primary font name as
presented to the users.

4) The name(s) of the Copyright Holder(s) or the Author(s) of the Font
Software shall not be used to promote, endorse or advertise any
Modified Version, except to acknowledge the contribution(s) of the
Copyright Holder(s) and the Author(s) or with their explicit written
permission.

5) The Font Software, modified or unmodified, in part or in whole,
must be distributed entirely under this license, and must not be
distributed under any other license. The requirement for fonts to
remain under this license does not apply to any document created using
the Font Software.

TERMINATION
This license becomes null and void if any of the above conditions are
not met.

DISCLAIMER
THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF COPYRIGHT, PATENT, TRADEMARK, OR OTHER RIGHT. IN NO EVENT SHALL THE
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM
OTHER DEALINGS IN THE FONT SOFTWARE.

## VisUI-derived default skin (audited; blocked; not incorporated)

Myra explicitly acknowledges VisUI as the source of its default skin. A
byte-level audit found 202 of 207 pinned Myra raw raster assets identical to
files at VisUI commit `4d5ce13b3bdef8c07a804c6a18e631fd3a64bc27`.
VisUI is Apache-2.0 licensed, while its `ui/NOTICE` additionally identifies
icons described as CC BY-ND 3.0 and points to a separate `ui/icons-license`
with further terms. No VisUI-derived asset is incorporated, and this entry is
not a redistribution approval. See `docs/default-assets-audit.md` and
`needs_human` P0-015b.

## Future dependencies

Notices for the selected font rasterizer and any future third-party dependency
must be added here before a source or binary is introduced.
