### Description

This example shows how to add user functions to a VTK shader. The example's fragment shader uses an implementation of [Ken Perlin's noise function](https://www.semanticscholar.org/paper/An-image-synthesizer-Perlin/e04d7772b91a83a901408eb0876bbb7814b1d4b5). Perlin received a Technical Achievement Academy Award for his work procedural texture.

The shader code contains a variable "k" that controls the frequency of the noisy colors. The default value will provide reasonable results for all polydata.

The file for the shader code is `src/Testing/Data/Shaders/PerlinNoise.glsl`.

!!! example "usage"
    BozoShader PerlinNoise.glsl horse.ply

!!! cite
    Perlin's original Siggraph Paper: [Perlin, K. 1985. "An Image Synthesizer." Computer Graphics 19(3)](https://dl.acm.org/doi/10.1145/325165.325247).

!!! cite
    This shader is inspired by Ken Perlin's Bozo shader.
