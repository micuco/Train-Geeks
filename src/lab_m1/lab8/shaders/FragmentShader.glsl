#version 330

// Input
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 eye_position;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// TODO(student): Declare any other uniforms
uniform vec3 light_position2;
uniform vec3 light_direction2;
uniform vec3 light_color2;

uniform int use_spot;
uniform float spot_cutoff;
uniform float spot_outer_cutoff;

uniform vec3 object_color;

// Output
layout(location = 0) out vec4 out_color;

vec3 PhongContribution(vec3 L, vec3 N, vec3 V, vec3 lightCol)
{
    L = normalize(L);
    N = normalize(N);
    V = normalize(V);

    float diff = max(dot(N, L), 0.0);

    float spec = 0.0;
    if (diff > 0.0) {
        vec3 R = reflect(-L, N);
        spec = pow(max(dot(V, R), 0.0), float(material_shininess));
    }

    vec3 diffuse = object_color * material_kd * diff * lightCol;
    vec3 specular = lightCol * material_ks * spec;

    return diffuse + specular;
}

float SpotFactor(vec3 L, vec3 spotDir, float spotCut, float spotOuter)
{
    vec3 Ldir = normalize(-L);
    vec3 Sdir = normalize(spotDir);

    float cosAngle = dot(Ldir, Sdir);

    if (spotOuter > 0.0) {
        if (cosAngle <= spotOuter) return 0.0;
        if (cosAngle >= spotCut) return 1.0;
        float t = (cosAngle - spotOuter) / (spotCut - spotOuter);
        return pow(t, 2.0);
    } else {
        if (cosAngle <= spotCut) return 0.0;
        float t = (cosAngle - spotCut) / (1.0 - spotCut);
        return pow(t, 6.0);
    }
}


void main()
{
    // TODO(student): Define ambient, diffuse and specular light components
    float ambient_light = 0.25;
    //float diffuse_light = 0;
    //float specular_light = 0;
    // It's important to distinguish between "reflection model" and
    // "shading method". In this shader, we are experimenting with the Phong
    // (1975) and Blinn-Phong (1977) reflection models, and we are using the
    // Phong (1975) shading method. Don't mix them up!
    vec3 N = normalize(world_normal);
    vec3 V = normalize(eye_position - world_position);

    vec3 ambient = 0.25 * object_color;

    vec3 L1 = light_position - world_position;
    float d1 = length(L1);
    vec3 contrib1 = PhongContribution(L1, N, V, light_color);

    if (use_spot == 1) {
        float sf1 = SpotFactor(L1, light_direction, spot_cutoff, spot_outer_cutoff);
        contrib1 *= sf1;
    }

    float att1 = 1.0 / (1.0 + 0.09 * d1 + 0.032 * d1 * d1);
    contrib1 *= att1;

    vec3 L2 = light_position2 - world_position;
    float d2 = length(L2);
    vec3 contrib2 = PhongContribution(L2, N, V, light_color2);

    if (use_spot == 1) {
        float sf2 = SpotFactor(L2, light_direction2, spot_cutoff, spot_outer_cutoff);
        contrib2 *= sf2;
    }

    float att2 = 1.0 / (1.0 + 0.09 * d2 + 0.032 * d2 * d2);
    contrib2 *= att2;

    vec3 final_color = ambient + contrib1 + contrib2;
    final_color = clamp(final_color, 0.0, 1.0);
    // TODO(student): If (and only if) the light is a spotlight, we need to do
    // some additional things.

    // TODO(student): Compute the total light. You can just add the components
    // together, but if you're feeling extra fancy, you can add individual
    // colors to the light components. To do that, pick some vec3 colors that
    // you like, and multiply them with the respective light components.

    // TODO(student): Write pixel out color
    out_color = vec4(final_color, 1.0);
}