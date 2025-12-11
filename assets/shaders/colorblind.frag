// Simple colorblind/contrast shader
// Modes are selected via a 3x3 color transform matrix and an optional contrast boost.

uniform sampler2D texture;
uniform mat3 colorMatrix;
uniform float contrast;
uniform float intensity;

void main() {
    vec4 src = texture2D(texture, gl_TexCoord[0].xy);
    vec3 transformed = colorMatrix * src.rgb;
    vec3 mixed = mix(src.rgb, transformed, intensity);
    // Apply contrast around 0.5
    mixed = (mixed - 0.5) * contrast + 0.5;
    gl_FragColor = vec4(clamp(mixed, 0.0, 1.0), src.a);
}
