#version 430 core

in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 upper_color1 = vec3(0.0);
uniform vec3 upper_color2 = vec3(1.0);

uniform vec3 lower_color1 = vec3(0.0);
uniform vec3 lower_color2 = vec3(1.0);

uniform bool horizontal = false;
uniform float time = 0.0;

const float MID_Y = 0.5;
const float Y = 0.4;
const float I = 0.2;

float x = TexCoords.x;
float y = TexCoords.y;

const float PI = 3.1415926538;
const float AMPLITUDE = 0.1;
const float FREQUENCY = 25;
const float SPEED = 0.1;

const float EDGE_BLUR = 0.075;

//Dither
const highp float NOISE_GRANULARITY = 0.5 / 255.0;
highp float random(highp vec2 coords) {
   return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453);
}

//Stacked sin waves
float stackedSin()
{
    float stackedSin = 0.0;
    stackedSin =  sin((x + time *  0.95 * SPEED) * FREQUENCY *  0.11)   * AMPLITUDE *  -0.99;
    stackedSin += sin((x + time *  0.64 * SPEED) * FREQUENCY * -0.17)   * AMPLITUDE *   0.98;
    stackedSin += sin((x + time * -0.33 * SPEED) * FREQUENCY *  0.23)   * AMPLITUDE *  -0.97;
    return stackedSin;
}

void main()
{
    vec3 color = vec3(1.0);

    vec3 upper_color = mix(upper_color1, upper_color2, y/1.0);
    vec3 lower_color = mix(lower_color1, lower_color2, y/1.0);

    float curve_y = MID_Y + stackedSin();
    float dist = curve_y - y;
    float alpha = smoothstep(-EDGE_BLUR * x, EDGE_BLUR * x, dist);

    color = mix(lower_color, upper_color, alpha);
    // Prevent color banding
    color += mix(-NOISE_GRANULARITY, NOISE_GRANULARITY, random(TexCoords));

    FragColor = vec4(color, 1.0);
}