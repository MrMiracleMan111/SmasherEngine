#version 130 // Or a higher version if your system supports it

uniform float u_time;
uniform vec2 u_resolution;

void main() {
    vec2 st = gl_FragCoord.xy / u_resolution.xy; // Normalized screen coordinates (0 to 1)

    // Simple color animation based on time
    vec3 color = vec3(abs(sin(u_time + st.x)), abs(cos(u_time + st.y)), 0.5);

    gl_FragColor = vec4(color, 1.0); // Output the final color
}