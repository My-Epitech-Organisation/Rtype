// Damage vignette shader - radial red gradient from edges to center
uniform float intensity;  // 0.0 = invisible, 1.0 = full effect
uniform vec2 resolution;  // Screen resolution

void main() {
    // Normalize coordinates to 0-1 range
    vec2 uv = gl_FragCoord.xy / resolution;
    
    // Calculate distance from center (0,0 at center, 1 at corners)
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(uv, center) * 2.0;  // Scale so edges are ~1.0
    
    // Create vignette effect - stronger at edges
    float vignette = smoothstep(0.3, 1.2, dist);
    
    // Apply intensity and color
    float alpha = vignette * intensity * 0.7;  // Max alpha 0.7
    
    gl_FragColor = vec4(1.0, 0.0, 0.0, alpha);  // Red with calculated alpha
}
