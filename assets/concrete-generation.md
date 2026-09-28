# Original concrete texture

`concrete-v1.png` was generated for cna-backrooms with the built-in OpenAI image generation tool on 2026-09-28. No reference image was supplied. The source PNG is retained without offline edits; SHA-256 is `992e46780dfc3dd3ecd5d6b32826ed87bb23cc3e7d60095146834fe1534cbb71`.

CNA decodes/resizes it to 1024 by 1024 and the game builds the existing mip chain. Wall and floor slots share the same GPU texture, with independently tinted/lit geometry at a two-metre footprint. Separate procedural wall/floor materials remain when the image is absent or undecodable. Nine wall comparisons, a second wall/floor round, three close floor views, 24 distant directions and controller views were inspected. Actual missing and corrupt-file launches verify both fallbacks. No offline image processing is used.

## Generation prompt

> Use case: photorealistic-natural.
>
> Asset type: seamless square diffuse/albedo texture for concrete walls in a small 3D Backrooms service-tunnel exploration game.
>
> Primary request: an original aged interior cast-concrete wall material, scanned straight on, orthographic and perfectly flat, filling the entire square edge to edge. A continuous approximately 2 by 2 metre patch of old grey concrete with small aggregate, fine pores, faint uneven dusty limewash and modest ingrained grime. Subtle irregular wear at 2–15 cm scale, very fine rough mineral texture, restrained hairline cracks and a few tiny pits. Muted warm-neutral grey average approximately RGB 150,146,136; low contrast, quiet natural variation. It should read as old concrete in a building service corridor, not cloud noise or smooth CGI plaster.
>
> Lighting: completely even diffuse albedo, no baked directional shadows, no vignette, no bright center, no metallic highlights.
>
> Constraints: seamless wrap on all four edges; no perspective, no floor, no ceiling, no objects, no pipes, no panels or brick grid, no visible wall boundary, no text or watermark. No large distinctive stains or repeated circular rust spots; no dramatic decay, rubble or horror illustrations. One square material texture only.
