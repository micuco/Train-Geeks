#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Uniforms for light properties
uniform vec3 light_position;
uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

uniform vec3 object_color;

// Output value to fragment shader
out vec3 color;


void main()
{
    // TODO(student): Compute world space vectors
    vec3 world_pos = vec3(Model * vec4(v_position, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(Model)));
    vec3 N = normalize(normalMatrix * v_normal);
    vec3 L = normalize(light_position - world_pos);
    vec3 V = normalize(eye_position - world_pos);

    // TODO(student): Define ambient light component
    float ambient_light = 0.25;
    vec3 ambient = ambient_light * object_color;

    // TODO(student): Compute diffuse light component
    float diffuse_light = max(dot(N, L), 0.0);
    vec3 diffuse = material_kd * diffuse_light * object_color;

    // TODO(student): Compute specular light component
    float specular_light = 0.0;

    // It's important to distinguish between "reflection model" and
    // "shading method". In this shader, we are experimenting with the Phong
    // (1975) and Blinn-Phong (1977) reflection models, and we are using the
    // Gouraud (1971) shading method. There is also the Phong (1975) shading
    // method, which we'll use in the future. Don't mix them up!
    if (diffuse_light > 0.0)
    {
        vec3 R = reflect(-L, N);
        specular_light = pow(max(dot(R,V), 0.0), material_shininess);
    }

    vec3 specular = material_ks * specular_light * vec3(1.0);

    // TODO(student): Compute light
    vec3 final_color = ambient + diffuse + specular;

    // TODO(student): Send color light output to fragment shader
    color = final_color;

    gl_Position = Projection * View * Model * vec4(v_position, 1.0);
}
